/* from http://www.libpng.org/pub/png/apps/pngquant.html */

#include "global.h"
#include "RageSurfaceUtils_Palettize.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageUtil.h"

typedef uint8_t pixval;
typedef uint8_t apixel[4];

#define PAM_GETR(p) ((p)[0])
#define PAM_GETG(p) ((p)[1])
#define PAM_GETB(p) ((p)[2])
#define PAM_GETA(p) ((p)[3])
#define PAM_ASSIGN(p,red,grn,blu,alf) \
   do { (p)[0] = (red); (p)[1] = (grn); (p)[2] = (blu); (p)[3] = (alf); } while (0)
#define PAM_EQUAL(p,q) \
   ((p)[0] == (q)[0] && (p)[1] == (q)[1] && (p)[2] == (q)[2] && (p)[3] == (q)[3])
#define PAM_DEPTH(newp,p,oldmaxval,newmaxval) \
   PAM_ASSIGN( (newp), \
      ( (uint8_t) PAM_GETR(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
      ( (uint8_t) PAM_GETG(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
      ( (uint8_t) PAM_GETB(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
      ( (uint8_t) PAM_GETA(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval) )

struct acolorhist_item
{
    apixel acolor;
    int value;
};

typedef struct acolorhist_list_item *acolorhist_list;
struct acolorhist_list_item
{
    struct acolorhist_item ch;
    acolorhist_list next;
};

struct acolorhash_hash
{
	enum { HASH_SIZE = 20023 };
	acolorhist_list hash[HASH_SIZE];
	acolorhash_hash()
	{
		ZERO( hash );
	}

	~acolorhash_hash()
	{
		for ( int i = 0; i < HASH_SIZE; ++i )
		{
			acolorhist_list achl, achlnext;
			for ( achl = hash[i]; achl != NULL; achl = achlnext )
			{
				achlnext = achl->next;
				free( (char*) achl );
			}
		}
	}
};



#define MAXCOLORS  32767
#define FS_SCALE   1024     /* Floyd-Steinberg scaling factor */

/* #define REP_AVERAGE_COLORS */
#define REP_AVERAGE_PIXELS



static acolorhist_item *mediancut( acolorhist_item *achv, int colors, int sum, int maxval, int newcolors );

static bool redcompare( const acolorhist_item &ch1, const acolorhist_item &ch2 )
{
	return PAM_GETR( ch1.acolor ) < PAM_GETR( ch2.acolor );
}

static bool greencompare( const acolorhist_item &ch1, const acolorhist_item &ch2 )
{
	return PAM_GETG( ch1.acolor ) < PAM_GETG( ch2.acolor );
}

static bool bluecompare( const acolorhist_item &ch1, const acolorhist_item &ch2 )
{
	return PAM_GETB( ch1.acolor ) < PAM_GETB( ch2.acolor );
}

static bool alphacompare( const acolorhist_item &ch1, const acolorhist_item &ch2 )
{
	return PAM_GETA( ch1.acolor ) < PAM_GETA( ch2.acolor );
}

static acolorhist_item *pam_computeacolorhist( const RageSurface *src, int maxacolors, int* acolorsP );
static void pam_addtoacolorhash( acolorhash_hash &acht, const uint8_t acolorP[4], int value );
static int pam_lookupacolor( const acolorhash_hash &acht, const uint8_t acolorP[4] );
static void pam_freeacolorhist( acolorhist_item *achv );

struct error_t
{
	int c[4];
};

void RageSurfaceUtils::Palettize( RageSurface *&pImg, int iColors, bool bDither )
{
	ASSERT( iColors != 0 );

    acolorhist_item *acolormap=NULL;
    int newcolors = 0;

	/* "apixel", etc. make assumptions about byte order. */
	RageSurfaceUtils::ConvertSurface( pImg, pImg->w, pImg->h, 32,
		Swap32BE(0xFF000000), Swap32BE(0x00FF0000), Swap32BE(0x0000FF00), Swap32BE(0x000000FF));

    pixval maxval = 255;

	{
        /*
         * Attempt to make a histogram of the colors, unclustered.
         * If at first we don't succeed, lower maxval to increase color
         * coherence and try again.  This will eventually terminate, with
         * maxval at worst 15, since 32^3 is approximately MAXCOLORS.
         *        [GRR POSSIBLE BUG:  what about 32^4 ?]
         */
		acolorhist_item *achv;
	    int colors;
        while(1)
		{
            achv = pam_computeacolorhist( pImg, MAXCOLORS, &colors );
            if( achv != NULL )
                break;
            pixval newmaxval = maxval / 2;

            for( int row = 0; row < pImg->h; ++row )
			{
				apixel *pP = (apixel *) (pImg->pixels+row*pImg->pitch);
                for( int col = 0; col < pImg->w; ++col, ++pP )
                    PAM_DEPTH( *pP, *pP, maxval, newmaxval );
			}
            maxval = newmaxval;
        }
        newcolors = min( colors, iColors );

        /* Apply median-cut to histogram, making the new acolormap. */
        acolormap = mediancut( achv, colors, pImg->h * pImg->w, maxval, newcolors );
        pam_freeacolorhist( achv );
    }

	RageSurface *pRet = CreateSurface( pImg->w, pImg->h, 8, 0, 0, 0, 0 );
	pRet->format->palette->ncolors = newcolors;

	/* Rescale the palette colors to a maxval of 255. */
	{
		RageSurfacePalette *pal = pRet->format->palette;
		for( int x = 0; x < pal->ncolors; ++x )
		{
			/* This is really just PAM_DEPTH() broken out for the palette. */
			pal->colors[x].r
				= (PAM_GETR(acolormap[x].acolor)*255 + (maxval >> 1)) / maxval;
			pal->colors[x].g
				= (PAM_GETG(acolormap[x].acolor)*255 + (maxval >> 1)) / maxval;
			pal->colors[x].b
				= (PAM_GETB(acolormap[x].acolor)*255 + (maxval >> 1)) / maxval;
			pal->colors[x].a
				= (PAM_GETA(acolormap[x].acolor)*255 + (maxval >> 1)) / maxval;
		}
	}

	/* Map the colors in the image to their closest match in the new colormap. */
	acolorhash_hash acht;

	bool fs_direction = 0;
	error_t *thiserr = NULL, *nexterr = NULL;

	if( bDither )
	{
		/* Initialize Floyd-Steinberg error vectors. */
		thiserr = new error_t[pImg->w + 2];
		nexterr = new error_t[pImg->w + 2];

		memset( thiserr, 0, sizeof(error_t) * (pImg->w + 2) );
	}

	for( int row = 0; row < pImg->h; ++row )
	{
		if( bDither )
			memset( nexterr, 0, sizeof(error_t) * (pImg->w + 2) );

		int col, limitcol;
		if( !fs_direction )
		{
			col = 0;
			limitcol = pImg->w;
		} else {
			col = pImg->w - 1;
			limitcol = -1;
		}

		const uint8_t *pIn = pImg->pixels + row*pImg->pitch;
		uint8_t *pOut = pRet->pixels + row*pRet->pitch;
		pIn += col * 4;
		pOut += col;

		do
		{
			int32_t sc[4];
			uint8_t pixel[4] = { pIn[0], pIn[1], pIn[2], pIn[3] };
			if( bDither )
			{
				/* Use Floyd-Steinberg errors to adjust actual color. */
				for( int c = 0; c < 4; ++c )
				{
					sc[c] = pixel[c] + thiserr[col + 1].c[c] / FS_SCALE;
					sc[c] = clamp( sc[c], 0, (int32_t) maxval );
				}

				PAM_ASSIGN( pixel, (uint8_t)sc[0], (uint8_t)sc[1], (uint8_t)sc[2], (uint8_t)sc[3] );
			}

			/* Check hash table to see if we have already matched this color. */
			int ind = pam_lookupacolor( acht, pixel );
			if( ind == -1 )
			{
				/* No; search acolormap for closest match. */
				long dist = 2000000000;
				for( int i = 0; i < newcolors; ++i )
				{
					const uint8_t *colors2 = acolormap[i].acolor;

					int newdist = 0;
					for( int c = 0; c < 4; ++c )
						newdist += ( int(pixel[c]) - colors2[c] ) * ( int(pixel[c]) - colors2[c] );

					if( newdist < dist )
					{
						ind = i;
						dist = newdist;
					}
				}

				pam_addtoacolorhash( acht, pixel, ind );
			}

			if( bDither )
			{
				/* Propagate Floyd-Steinberg error terms. */
				if( !fs_direction )
				{
					for( int c = 0; c < 4; ++c )
					{
						long err = (sc[c] - (long)acolormap[ind].acolor[c])*FS_SCALE;
						thiserr[col + 2].c[c] += ( err * 7 ) / 16;
						nexterr[col    ].c[c] += ( err * 3 ) / 16;
						nexterr[col + 1].c[c] += ( err * 5 ) / 16;
						nexterr[col + 2].c[c] += ( err * 1 ) / 16;
					}
				} else {
					for( int c = 0; c < 4; ++c )
					{
						long err = (sc[c] - (long)acolormap[ind].acolor[c])*FS_SCALE;
						thiserr[col    ].c[c] += ( err * 7 ) / 16;
						nexterr[col + 2].c[c] += ( err * 3 ) / 16;
						nexterr[col + 1].c[c] += ( err * 5 ) / 16;
						nexterr[col    ].c[c] += ( err * 1 ) / 16;
					}
				}
			}

            *pOut = (uint8_t) ind;

            if( !fs_direction )
			{
                ++col;
                pIn += 4;
                ++pOut;
            } else {
                --col;
                pIn -= 4;
                --pOut;
            }
        }
        while( col != limitcol );

		if( bDither )
		{
			swap( thiserr, nexterr );
			fs_direction = !fs_direction;
		}
	}

	delete [] thiserr;
	delete [] nexterr;

	delete pImg;
	pImg = pRet;
}



/*
 * Here is the fun part, the median-cut colormap generator.  This is based
 * on Paul Heckbert's paper, "Color Image Quantization for Frame Buffer
 * Display," SIGGRAPH 1982 Proceedings, page 297.
 */

typedef struct box *box_vector;
struct box
{
    int ind;
    int colors;
    int sum;
};

static bool CompareBySumDescending( const box &b1, const box &b2 )
{
	return b2.sum < b1.sum;
}


static acolorhist_item *mediancut( acolorhist_item *achv, int colors, int sum, int maxval, int newcolors )
{
	acolorhist_item *acolormap;
	box_vector bv;
	int boxes;

	bv = (box_vector) malloc( sizeof(struct box) * newcolors );
	ASSERT( bv );
	acolormap = (acolorhist_item*) malloc( sizeof(struct acolorhist_item) * newcolors);
	ASSERT( acolormap );

	for ( int i = 0; i < newcolors; ++i )
		PAM_ASSIGN( acolormap[i].acolor, 0, 0, 0, 0 );

	/* Set up the initial box. */
	bv[0].ind = 0;
	bv[0].colors = colors;
	bv[0].sum = sum;
	boxes = 1;

	/* Main loop: split boxes until we have enough. */
	while( boxes < newcolors )
	{
		int indx, clrs;
		int sm;
		int minr, maxr, ming, mina, maxg, minb, maxb, maxa, v;
		int halfsum, lowersum;

		/* Find the first splittable box. */
		int bi;
		for( bi = 0; bi < boxes; ++bi )
			if ( bv[bi].colors >= 2 )
				break;
		if( bi == boxes )
			break;        /* ran out of colors! */
        indx = bv[bi].ind;
        clrs = bv[bi].colors;
        sm = bv[bi].sum;

		/*
		 * Go through the box finding the minimum and maximum of each
		 * component - the boundaries of the box.
		 */
		minr = maxr = PAM_GETR( achv[indx].acolor );
		ming = maxg = PAM_GETG( achv[indx].acolor );
		minb = maxb = PAM_GETB( achv[indx].acolor );
		mina = maxa = PAM_GETA( achv[indx].acolor );
		for ( int i = 1; i < clrs; ++i )
		{
			v = PAM_GETR( achv[indx + i].acolor );
			if ( v < minr ) minr = v;
			if ( v > maxr ) maxr = v;
			v = PAM_GETG( achv[indx + i].acolor );
			if ( v < ming ) ming = v;
			if ( v > maxg ) maxg = v;
			v = PAM_GETB( achv[indx + i].acolor );
			if ( v < minb ) minb = v;
			if ( v > maxb ) maxb = v;
			v = PAM_GETA( achv[indx + i].acolor );
			if ( v < mina ) mina = v;
			if ( v > maxa ) maxa = v;
		}

		/* Find the largest dimension, and sort by that component. */
		if ( maxa - mina >= maxr - minr && maxa - mina >= maxg - ming && maxa - mina >= maxb - minb )
			sort( &achv[indx], &achv[indx+clrs], alphacompare );
		else if ( maxr - minr >= maxg - ming && maxr - minr >= maxb - minb )
			sort( &achv[indx], &achv[indx+clrs], redcompare );
		else if ( maxg - ming >= maxb - minb )
			sort( &achv[indx], &achv[indx+clrs], greencompare );
		else
			sort( &achv[indx], &achv[indx+clrs], bluecompare );

		/*
		 * Now find the median based on the counts, so that about half the
		 * pixels (not colors, pixels) are in each subdivision.
		 */
		lowersum = achv[indx].value;
		halfsum = sm / 2;
		int j;
		for ( j = 1; j < clrs - 1; ++j )
		{
			if ( lowersum >= halfsum )
				break;
			lowersum += achv[indx + j].value;
		}

		/* Split the box, and sort to bring the biggest boxes to the top. */
		bv[bi].colors = j;
		bv[bi].sum = lowersum;
		bv[boxes].ind = indx + j;
		bv[boxes].colors = clrs - j;
		bv[boxes].sum = sm - lowersum;
		++boxes;
		sort( &bv[0], &bv[boxes], CompareBySumDescending );
	}

	/*
	 * Ok, we've got enough boxes.  Now choose a representative color for
	 * each box.  There are a number of possible ways to make this choice.
	 * One would be to choose the center of the box; this ignores any structure
	 * within the boxes.  Another method would be to average all the colors in
	 * the box - this is the method specified in Heckbert's paper.  A third
	 * method is to average all the pixels in the box.  You can switch which
	 * method is used by switching the commenting on the REP_ defines at
	 * the beginning of this source file.
	 */
	for( int bi = 0; bi < boxes; ++bi )
	{
#ifdef REP_AVERAGE_COLORS
		int indx = bv[bi].ind;
		int clrs = bv[bi].colors;
		long r = 0, g = 0, b = 0, a = 0;

		for ( i = 0; i < clrs; ++i )
		{
			r += PAM_GETR( achv[indx + i].acolor );
			g += PAM_GETG( achv[indx + i].acolor );
			b += PAM_GETB( achv[indx + i].acolor );
			a += PAM_GETA( achv[indx + i].acolor );
		}
		r = r / clrs;
		g = g / clrs;
		b = b / clrs;
		a = a / clrs;
		PAM_ASSIGN( acolormap[bi].acolor, r, g, b, a );
#endif /*REP_AVERAGE_COLORS*/
#ifdef REP_AVERAGE_PIXELS
		int indx = bv[bi].ind;
		int clrs = bv[bi].colors;
		long r = 0, g = 0, b = 0, a = 0, sum = 0;

		for ( int i = 0; i < clrs; ++i )
		{
			r += PAM_GETR( achv[indx + i].acolor ) * achv[indx + i].value;
			g += PAM_GETG( achv[indx + i].acolor ) * achv[indx + i].value;
			b += PAM_GETB( achv[indx + i].acolor ) * achv[indx + i].value;
			a += PAM_GETA( achv[indx + i].acolor ) * achv[indx + i].value;
			sum += achv[indx + i].value;
		}
		r = r / sum;
		r = min( r, (long) maxval );
		g = g / sum;
		g = min( g, (long) maxval );
		b = b / sum;
		b = min( b, (long) maxval );
		a = a / sum;
		a = min( a, (long) maxval );
		PAM_ASSIGN( acolormap[bi].acolor, (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a );
#endif /*REP_AVERAGE_PIXELS*/
	}

	/* All done. */
	return acolormap;
}

/*
 * libpam3.c - pam (portable alpha map) utility library part 3
 *
 * Colormap routines.
 *
 * Copyright (C) 1989, 1991 by Jef Poskanzer.
 * Copyright (C) 1997 by Greg Roelofs.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 */

#define HASH_SIZE 20023

#define pam_hashapixel(p) ( ( ( (long) PAM_GETR(p) * 33023 + \
                                (long) PAM_GETG(p) * 30013 + \
                                (long) PAM_GETB(p) * 27011 + \
                                (long) PAM_GETA(p) * 24007 ) \
                              & 0x7fffffff ) % HASH_SIZE )

static bool pam_computeacolorhash( const RageSurface *src, int maxacolors, int* acolorsP, acolorhash_hash &hash )
{
	ASSERT( src->format->BytesPerPixel == 4 );

	*acolorsP = 0;

	/* Go through the entire image, building a hash table of colors. */
	for( int row = 0; row < src->h; ++row )
	{
		const apixel *pP = (const apixel *) (src->pixels + row*src->pitch);
		for( int col = 0; col < src->w; ++col, pP++ )
		{
			int hashval = pam_hashapixel( *pP );
			acolorhist_list achl;
			for ( achl = hash.hash[hashval]; achl != NULL; achl = achl->next )
				if ( PAM_EQUAL( achl->ch.acolor, *pP ) )
					break;
			if ( achl != NULL )
				++achl->ch.value;
			else
			{
				if ( ++(*acolorsP) > maxacolors )
					return false;
				achl = (acolorhist_list) malloc( sizeof(struct acolorhist_list_item) );
				ASSERT( achl != NULL );

				memcpy( achl->ch.acolor, *pP, sizeof(apixel) );
				achl->ch.value = 1;
				achl->next = hash.hash[hashval];
				hash.hash[hashval] = achl;
			}
		}
	}

	return true;
}

static acolorhist_item *pam_acolorhashtoacolorhist( const acolorhash_hash &acht, int maxacolors )
{
	/* Collate the hash table into a simple acolorhist array. */
	acolorhist_item *achv = (acolorhist_item*) malloc( maxacolors * sizeof(struct acolorhist_item) );
	ASSERT( achv != NULL );

	/* Loop through the hash table. */
	int j = 0;
	for ( int i = 0; i < HASH_SIZE; ++i )
	{
		for ( acolorhist_list achl = acht.hash[i]; achl != NULL; achl = achl->next )
		{
			/* Add the new entry. */
			achv[j] = achl->ch;
			++j;
		}
	}

	/* All done. */
	return achv;
}

static acolorhist_item *pam_computeacolorhist( const RageSurface *src, int maxacolors, int* acolorsP )
{
	acolorhash_hash acht;
	if ( !pam_computeacolorhash( src, maxacolors, acolorsP, acht ) )
		return NULL;

	acolorhist_item *achv = pam_acolorhashtoacolorhist( acht, maxacolors );
	return achv;
}

static void pam_addtoacolorhash( acolorhash_hash &acht, const uint8_t acolorP[4], int value )
{
	acolorhist_list achl = (acolorhist_list) malloc( sizeof(struct acolorhist_list_item) );
	ASSERT( achl != NULL );

	int hash = pam_hashapixel( acolorP );
	memcpy( achl->ch.acolor, acolorP, sizeof(apixel) );
	achl->ch.value = value;
	achl->next = acht.hash[hash];
	acht.hash[hash] = achl;
}


static int pam_lookupacolor( const acolorhash_hash &acht, const uint8_t acolorP[4] )
{
	const int hash = pam_hashapixel( acolorP );
	for ( acolorhist_list_item *achl = acht.hash[hash]; achl != NULL; achl = achl->next )
		if ( PAM_EQUAL( achl->ch.acolor, acolorP ) )
			return achl->ch.value;

	return -1;
}


static void pam_freeacolorhist( acolorhist_item *achv )
{
	free( (char*) achv );
}

/*
 * Copyright (C) 1989, 1991 by Jef Poskanzer.
 * Copyright (C) 1997, 2000, 2002 by Greg Roelofs; based on an idea by
 *                                Stefan Schneider.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 */
