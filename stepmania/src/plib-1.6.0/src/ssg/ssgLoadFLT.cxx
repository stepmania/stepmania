/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id$
*/

//
//  $Id$
//
//
//    OpenFlight loader for SSG
//
//
//  The loading of FLT files can to some degree be controlled by 
//  setting environment variables:
//     
//     FLTNOCLEAN   - if set disables all kinds of scene graph optimisations
//     FLTNOEXT     - ignore external references
//     FLTNOTEX     - disables textures altogether
//     FLTNOMIPMAP  - configure textures for bilinear filtering (no mipmaps)
//
//
//  Some known issues:
//
//  1. The color table in v14.0 and earlier is defined differently and
//     cannot be parsed. It may consist of 16 colours of 16 brightnesses
//     each, and the RGB values are passed as shorts.
//
//  2. Geometry in v11.0 and earlier are defined using obsolete records
//     (opcodes 6, 7, 8 and 9). Need specifications to parse them.
//
//  3. How to find external files properly. Some loaders (most notably 
//     IRIS Performer and MultiGen) uses the variable FLTEXTERNPATH.
//     (MultiGen also uses FLTPATH and TXTPATH I think).
//
//  4. How to do mmap on Mac and PS2.
//
//  5. No optimisations are done on the geometry. New triangle strip
//     generation code planned for SSG, and should be used when ready.
//
//  6. Optimisation done on the scene graph level are minimal. 
//
//
//
/*
 * Another OpenFlight loader for Performer
 * Copyright (C) 1998-2000  Marten Stromberg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "ssg.h"
#include "ssgLoaderWriterStuff.h"

/* define this to compile against older versions of SSG */
/*#define NO_LOADER_OPTIONS*/

/* debug */
/*#define NO_LIGHTING*/
/*#define NO_TEXTURES*/
/*#define NO_COLORS*/

/* Try to figure out how to do mmap. */
#ifdef _WIN32
# define USE_WIN32_MMAP
#elif defined(__sgi) || defined(__GNUC__) /* OK? Any Unix dialect should work. */
# include <unistd.h>
# ifdef _POSIX_MAPPED_FILES
#  define USE_POSIX_MMAP
# endif
#endif

#define USE_ALLOCA

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef __sgi
# include <sys/endian.h>
# ifdef USE_ALLOCA
#  include <alloca.h>
# endif
#endif
#ifdef USE_POSIX_MMAP
# include <time.h>
# include <sys/time.h>   /* Need both for Mandrake 8.0 !! */
# include <sys/stat.h>
# include <sys/mman.h>
#endif
#ifdef _MSC_VER
# include <io.h>
# ifdef USE_ALLOCA
#  include <malloc.h>
# endif
#endif

#ifdef __MWERKS__
#  ifdef USE_ALLOCA
#    include <alloca.h>
#  endif
#endif

#ifdef __BORLANDC__
# ifdef USE_ALLOCA
#  include <malloc.h>
# endif
#endif

#if defined(__MINGW32__)
# ifdef USE_ALLOCA
#  include <libiberty.h>
# endif
#endif


#ifndef O_BINARY
# define O_BINARY 0
#endif

#ifndef R_OK
# define R_OK 4
#endif

#ifndef ABS
# define ABS(x) ((x)>=0?(x):-(x))
#endif
#ifndef MIN
# define MIN(a,b) ((a)<=(b)?(a):(b))
#endif
#ifndef MAX
# define MAX(a,b) ((a)>=(b)?(a):(b))
#endif
#define MIN3(a,b,c) ((a)<=(b)?MIN(a,c):MIN(b,c))
#define MAX3(a,b,c) ((a)>=(b)?MAX(a,c):MAX(b,c))
#define CLAMP(x,lo,hi) ((x)<=(lo)?(lo):(x)>=(hi)?(hi):(x))

#define template _template /* trams */

typedef unsigned char ubyte;

#ifdef _WIN32
typedef unsigned short ushort;
typedef unsigned int uint;
#endif

// 525 = negative identation, 539= did not expect positive identation
//lint -save -e525 -e539

/*
 * byte sex 
 */

/* XXX what about PDP_ENDIAN? */

/* Help! Is this correct? */
#if (!defined(BYTE_ORDER) && defined(WIN32) && !defined(NOT_INTEL_BYTE_ORDER))
# define LITTLE_ENDIAN  1234
# define BIG_ENDIAN     4321
# define BYTE_ORDER     LITTLE_ENDIAN
#endif

/* #undef BYTE_ORDER */

#if !defined(BYTE_ORDER) || BYTE_ORDER != BIG_ENDIAN

#ifdef PROBE_ENDIAN
static void _swab16(const void *src, void *dst, int n)
{
   ushort *s = (ushort *)src;
   ushort *d = (ushort *)dst;
   while (n--) {
      ushort t = *s++;
      *d++ = (((t & 0xff00U) >> 8) |
	      ((t & 0x00ffU) << 8));
   }
}
#endif

static void _swab32(const void *src, void *dst, int n)
{
   uint *s = (uint *)src;
   uint *d = (uint *)dst;
   while (n--) {
      uint t = *s++;
      *d++ = (((t & 0xff000000U) >> 24) |
	      ((t & 0x00ff0000U) >> 8) |
	      ((t & 0x0000ff00U) << 8) |
	      ((t & 0x000000ffU) << 24));
   }
}

static void _swab64(const void *src, void *dst, int n)
{
   /* XXX how to check if 64-bit integers are available?? */
   uint *s = (uint *)src;
   uint *d = (uint *)dst;
   while (n--) {
      uint t0 = *s++;
      uint t1 = *s++;
      *d++ = (((t1 & 0xff000000U) >> 24) |
	      ((t1 & 0x00ff0000U) >> 8) |
	      ((t1 & 0x0000ff00U) << 8) |
	      ((t1 & 0x000000ffU) << 24));
      *d++ = (((t0 & 0xff000000U) >> 24) |
	      ((t0 & 0x00ff0000U) >> 8) |
	      ((t0 & 0x0000ff00U) << 8) |
	      ((t0 & 0x000000ffU) << 24));
   }
}

#endif

#if !defined(BYTE_ORDER) || BYTE_ORDER != LITTLE_ENDIAN

static void _copy16(const void *src, void *dst, int n)
{
   memcpy(dst, src, 2*n);
}

static void _copy32(const void *src, void *dst, int n)
{
   memcpy(dst, src, 4*n);
}

static void _copy64(const void *src, void *dst, int n)
{
   memcpy(dst, src, 8*n);
}

#endif


#if defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN

/* big endian */

inline uint get16u(const void *ptr)
{
   return *(ushort *)ptr;
}

inline uint get32u(const void *ptr)
{
   return *(uint *)ptr;
}

#define get16v _copy16
#define get32v _copy32
#define get64v _copy64

#elif defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN

/* little endian */

inline uint get16u(const void *ptr)
{
   ushort tmp = *(ushort *)ptr;
   return (((tmp & 0xff00U) >> 8) |
	   ((tmp & 0x00ffU) << 8));
}

inline uint get32u(const void *ptr)
{
   uint tmp = *(uint *)ptr;
   return (((tmp & 0xff000000U) >> 24) |
	   ((tmp & 0x00ff0000U) >> 8) |
	   ((tmp & 0x0000ff00U) << 8) |
	   ((tmp & 0x000000ffU) << 24));
}

#define get16v _swab16
#define get32v _swab32
#define get64v _swab64

#else

/* any endian */

inline uint get16u(const void *ptr)
{
   ubyte *u = (ubyte *)ptr;
   return (((uint)u[0] << 8) | 
	   ((uint)u[1] << 0));
}

inline uint get32u(const void *ptr)
{
   ubyte *u = (ubyte *)ptr;
   return (((uint)u[0] << 24) | 
	   ((uint)u[1] << 16) | 
	   ((uint)u[2] << 8) | 
	   ((uint)u[3] << 0));
}

static void (*get16v)(const void *, void *, int);
static void (*get32v)(const void *, void *, int);
static void (*get64v)(const void *, void *, int);

#define PROBE_ENDIAN 1

#endif

#define get16i(ptr) (short)get16u(ptr)
#define get32i(ptr) (int)get32u(ptr)


/* splay trees
 *
 * for further details, see "Self-adjusting Binary Search Trees"
 * by Sleator and Tarjan, JACM Volume 32, No 3, July 1985, pp 652-686.
 */

struct snode {
   struct snode *left, *right;
   void *key;
   void *data;
};

typedef int (*sfunc)(const void *key1, const void *key2);

static struct snode *splay(struct snode *t, const void *key, sfunc comp)
{
   struct snode N, *l, *r, *y;
     
   if (t == NULL) return t;
   N.left = N.right = NULL;
   l = r = &N;
 
   for (;;) {
      if (comp(key, t->key) < 0) {
	 if (t->left == NULL) break;
	 if (comp(key, t->left->key) < 0) {
	    y = t->left;                           /* rotate right */
	    t->left = y->right;
	    y->right = t;
	    t = y;
	    if (t->left == NULL) break;
	 }
	 r->left = t;                               /* link right */
	 r = t;
	 t = t->left;
      }
      else if (comp(key, t->key) > 0) {
	 if (t->right == NULL) break;
	 if (comp(key, t->right->key) > 0) {
	    y = t->right;                           /* rotate left */
	    t->right = y->left;
	    y->left = t;
	    t = y;
	    if (t->right == NULL) break;
	 }
	 l->right = t;                              /* link left */
	 l = t;
	 t = t->right;
      }
      else {
	 break;
      }
   }
   
   l->right = t->left;                              /* assemble */
   r->left = t->right;
   t->left = N.right;
   t->right = N.left;
   
   return t;
}

static struct snode *sinsert(struct snode *root, void *key, size_t size, sfunc comp)
{
   struct snode *t, *x;
   t = splay(root, key, comp);
   if (t != NULL && comp(t->key, key) == 0)
      return t;
   //x = (struct snode *)malloc(sizeof(struct snode));
   x = new snode;
   assert( x != NULL );
   if (t == NULL) {
      x->left = x->right = NULL;
   }
   else if (comp(key, t->key) < 0) {
      x->left = t->left;
      x->right = t;
      t->left = NULL;
   }
   else {
      x->right = t->right;
      x->left = t;
      t->right = NULL;
   }
   if (size > 0) {
      //x->key = malloc(size);
      x->key = new ubyte[size];
      memcpy(x->key, key, size);
   }
   else {
      x->key = key;
   }
   x->data = (void *)-1;
   return x;
}

#define S_KEY   1 /* free key */
#define S_DATA  2 /* free data */
#define S_TREE  4 /* delete scene graph (SSG special) */

static void deltree(ssgEntity *node)
{
   if (node->getRef() <= 1 && node->isAKindOf(ssgTypeBranch())) {
      ssgBranch *grp = (ssgBranch *)node;
      int n = grp->getNumKids();
      while (n--) {
	 deltree(grp->getKid(n));
	 grp->removeKid(n); // delete kid (iff ref == 0)
      }
   }
}

static void sfree(struct snode *x, int flags)
{
   if (x) {      
      sfree(x->left, flags);
      sfree(x->right, flags);
      if ((flags & S_KEY))
	 //free(x->key);
	 delete [] (ubyte *)x->key;
      if (x->data != (void *)-1 && x->data != 0) {
	 if ((flags & S_DATA)) 
	    //free(x->data);
	    delete [] (ubyte *)x->data;
	 if ((flags & S_TREE)) {
	    deltree((ssgEntity *)x->data);
	    ssgDeRefDelete((ssgEntity *)x->data);
	 }
      }
   }
}

static int ptrcmp(const void *key1, const void *key2)
{
   return (const char *)key1 - (const char *)key2;
}


static void hexdump(enum ulSeverity severity, const ubyte *buf, int size, int offset)
{
#define COLS 16
   char line[1024], *lp;
   while (size > 0) {
      int i, n = MIN(size, COLS);
      lp = line;
      lp += sprintf(lp, "%04x ", offset);
      for (i = 0; i < n; i++)
	 lp += sprintf(lp, " %02x", buf[i]);
      for (; i < COLS; i++)
	 lp += sprintf(lp, "   ");
      *lp++ = ' ';
      *lp++ = ' ';
      for (i = 0; i < n; i++)
	 *lp++ = (buf[i] & 0x7f) < 0x20 ? '.' : buf[i];
      *lp = 0;
      ulSetError(severity, line);
      buf += COLS;
      offset += COLS;
      size -= COLS;
   }
}



#define MAXDEPTH 256

#define UNPACK_ABGR(dst, src) \
     ((dst)[0] = (1.0f/255.0f)*(src)[3], \
      (dst)[1] = (1.0f/255.0f)*(src)[2], \
      (dst)[2] = (1.0f/255.0f)*(src)[1], \
      (dst)[3] = 1.0f)

#define UNPACK_ABGR2(dst, src, intensity) \
     ((dst)[0] = (1.0f/255.0f/127.0f)*(src)[3]*(intensity), \
      (dst)[1] = (1.0f/255.0f/127.0f)*(src)[2]*(intensity), \
      (dst)[2] = (1.0f/255.0f/127.0f)*(src)[1]*(intensity), \
      (dst)[3] = 1.0f)

/* triangle flags */
#define TRI_COLOR_MATERIAL 0x01  /* --> apply material on vertex colors */
#define TRI_TRANSLUCENT    0x02 
#define TRI_SUBFACE        0x04  /* offset geometry */
#define TRI_BBOARD_AXIAL   0x08
#define TRI_BBOARD_POINT   0x10
/*#define TRI_WIREFRAME      0x20*/
#define TRI_BBOARD (TRI_BBOARD_AXIAL | TRI_BBOARD_POINT)

/* vertex bind */
#define BIND_COLOR        0x01
#define BIND_NORMAL       0x02
#define BIND_TEXCOORD     0x04
#define BIND_FLAT_COLOR   0x08
#define BIND_FLAT_NORMAL  0x10
#define BIND_FLAT (BIND_FLAT_COLOR | BIND_FLAT_NORMAL) 

struct fltTriangle {

   ssgState *state;

   ubyte flags;
   ubyte bind;

#if 0
   short priority; /* affects the ordering within the geoset */
   ushort transparency; /* eventually applied to colors when done */
#endif
   
   int index[3];
   sgVec4 color; /* face color *or* material contributions to vertex colors */

};

struct fltTexture {
   char *file;
   ssgState *state; /* user-defined state */
   ssgTexture *tex;
   int alpha;
};

struct fltState {

   fltState() {
      memset(this, 0, sizeof(*this));  
      notex_state = (ssgState *)-1;
      atris = 256;
      //tris = (fltTriangle *)malloc(sizeof(fltTriangle) * atris);
      tris = new fltTriangle[atris];
   }

   ~fltState() {
      sfree(texs, S_DATA);
      sfree(mtls, S_DATA);
      sfree(refs, S_TREE);
      if (vtab) {
	 delete [] offset;
	 delete [] bind;
	 delete [] coord;
	 delete [] color;
	 delete [] normal;
	 delete [] texcoord;
      }
      //free(tris);
      delete [] tris;
   }

   const char *filename;
   int revision;
   int major;
   int minor;

   /* Vertex Table */
   ubyte *vtab; /* start of vertex table */
   int vnum;
   int *offset; /* chunk offset (used as index) */
   ubyte *bind; /* 1 - color, 2 - normal, 4 - texcoord */
   sgVec3 *coord;
   sgVec4 *color;
   sgVec3 *normal;
   sgVec2 *texcoord;

   /* Other Tables (these may be sparse, that is why arrays are not used) */
   struct snode *mtls; /* index --> float[14] */
   struct snode *texs; /* index --> fltTexture */
   struct snode *refs; /* index --> ssgEntity */
   ubyte (*ctab)[4]; /* packed ABGR color table */
   int cnum;
   ssgState *notex_state; /* special user-defined state if no texture */

   /* Collected Geometry */
   fltTriangle *tris;
   int ntris, atris;
   fltTriangle *temp;
   char *parent_name;
};

struct fltNodeAttr {   

   /* allocated using new/delete for convenience */
   fltNodeAttr() { memset(this, 0, sizeof(*this)); }
   ~fltNodeAttr() { if (name) /*free(name);*/ delete name; }

   /* properies that are not applied immediately */

   char *name;

   int replicate;
   int transform;
   int islod;

   sgMat4 mat;
   sgVec2 range;
   sgVec3 center;

};

static int Inited = 0;
static int NoTextures = 0;
static int NoMipmaps = 0;
static int NoExternals = 0;
static int NoClean = 0;

#ifndef NO_LOADER_OPTIONS
static ssgLoaderOptions *LoaderOptions;
#endif

static struct snode *TexCache; /* filename --> fltTexture */

static fltTexture *LoadTex(char *fname)
{
   TexCache = sinsert(TexCache, fname, strlen(fname) + 1, (sfunc)strcmp);
   if (TexCache->data == (void *)-1) {
      //fltTexture *tex = (fltTexture *)malloc(sizeof(fltTexture));
      fltTexture *tex = new fltTexture;
      assert ( tex != NULL );
      tex->file = fname;
#ifdef NO_LOADER_OPTIONS
      tex->state = 0;
      tex->tex = new ssgTexture(fname, 1, 1, !NoMipmaps);
#else
      tex->state = LoaderOptions->createState(fname);
      tex->tex = tex->state ? 0 : LoaderOptions->createTexture(fname, 1, 1, !NoMipmaps);
#endif
      tex->alpha = tex->tex ? tex->tex->hasAlpha() : 0;
      TexCache->data = tex;
   }
   return (fltTexture *)TexCache->data;
}

struct StateInfo {
   int cf; /* cull face */
   int tr; /* translucent */
   int cm; /* color material */
   ssgTexture *tex;
   float *mtl;
   float alpha;
};

static int StateCompare(const void *key1, const void *key2)
{
   const StateInfo *s1 = (const StateInfo *)key1;
   const StateInfo *s2 = (const StateInfo *)key2;
   int d;
   d = s1->cf - s2->cf;
   if (d == 0) {
      d = s1->tr - s2->tr;
      if (d == 0) {
	 d = s1->cm - s2->cm;
	 if (d == 0) {
	    d = (char *)s1->tex - (char *)s2->tex;
	    if (d == 0) {
	       if (s1->mtl == 0 || s2->mtl == 0)
		  d = (char *)s1->mtl - (char *)s2->mtl;
	       else {
		  int i = s1->cm ? 6 : 0;
		  for (; i < 12 && d == 0; i++)
		     if (s1->mtl[i] < s2->mtl[i] - 0.01f)
			d = -1;
		     else if (s1->mtl[i] > s2->mtl[i] + 0.01f)
			d = 1;
		  if (d == 0) {
		     if (s1->alpha < s2->alpha - 0.01f)
			d = -1;
		     else if (s1->alpha > s2->alpha + 0.01f)
			d = 1;
		  }
	       }
	    }
	 }
      }
   }
   return d;
}

static struct snode *StateCache; /* StateInfo --> ssgSimpleState */

static ssgSimpleState *ConstructState(StateInfo *key)
{
   StateCache = sinsert(StateCache, key, sizeof(StateInfo), StateCompare);
   if (StateCache->data == (void *)-1) {
      ssgSimpleState *s;

      /*ulSetError(UL_DEBUG, "new state --");*/

      s = new ssgSimpleState;

      if (key->cf) {
	 s->enable(GL_CULL_FACE);
      }
      else {
	 s->disable(GL_CULL_FACE);
	 /*ulSetError(UL_DEBUG, "  two-sided");*/
      }

      if (key->tr) {
	 s->setTranslucent();
	 s->enable(GL_BLEND); /* XXX what about multisampling? */
	 /*ulSetError(UL_DEBUG, "  transparent");*/
      }
      else {
	 s->setOpaque();
	 s->disable(GL_BLEND);
      }
      /* leave the alpha test to the application */

      if (key->mtl) {
	 float *m = key->mtl;
	 sgVec4 c;
	 /*ulSetError(UL_DEBUG, "%.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f  %d",
	   m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], key->cm);*/
	 if (key->cm) {
	    s->enable(GL_COLOR_MATERIAL);
	    s->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
	    //ulSetError(UL_DEBUG, "  color material");
	 }
	 else {
	    s->disable(GL_COLOR_MATERIAL);
	    sgSetVec4(c, m[0], m[1], m[2], key->alpha); s->setMaterial(GL_AMBIENT, c);
	    sgSetVec4(c, m[3], m[4], m[5], key->alpha); s->setMaterial(GL_DIFFUSE, c);
	    //ulSetError(UL_DEBUG, "  ambient   %.2f %.2f %.2f", m[0], m[1], m[2]);
	    //ulSetError(UL_DEBUG, "  diffuse   %.2f %.2f %.2f", m[3], m[4], m[5]);
	 }
	 sgSetVec4(c, m[6], m[7], m[8], key->alpha); s->setMaterial(GL_SPECULAR, c);
	 sgSetVec4(c, m[9], m[10], m[11], key->alpha); s->setMaterial(GL_EMISSION, c);
	 //ulSetError(UL_DEBUG, "  specular  %.2f %.2f %.2f", m[6], m[7], m[8]);
	 //ulSetError(UL_DEBUG, "  emission  %.2f %.2f %.2f", m[9], m[10], m[11]);
	 //ulSetError(UL_DEBUG, "  alpha %.2f", key->alpha);
	 s->setShininess(m[12]);
	 s->enable(GL_LIGHTING);
      }
      else {
	 s->disable(GL_LIGHTING);
	 /*ulSetError(UL_DEBUG, "  no lighting");*/
      }

      if (key->tex) {	 
	 s->enable(GL_TEXTURE_2D);
	 s->setTexture(key->tex);
	 /*ulSetError(UL_DEBUG, "  texture %s", key->tex->getFilename());*/
      }
      else {
	 s->disable(GL_TEXTURE_2D);
      }

      s->setShadeModel(GL_SMOOTH);
      
      s->ref();

      StateCache->data = s;
   }
   return (ssgSimpleState *)StateCache->data;
}

static int tricmp(const void *a, const void *b)
{
   /* this comparison is used to sort the list of triangles in order to 
    * generate as few geosets as possible.
    */
   fltTriangle *ta = (fltTriangle *)a;
   fltTriangle *tb = (fltTriangle *)b;
   int d = (char *)ta->state - (char *)tb->state;
   if (d == 0) {
      d = ta->flags - tb->flags;
      if (d == 0)
	 d = ta->bind - tb->bind;
   }
   return d;
}

#if 0 /* no longer needed since this is now the default blend equation */
static int PreDrawTranslucent(ssgEntity *)
{
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   return 1;
}
#endif

static int PreDrawSubface(ssgEntity *)
{
#ifdef GL_VERSION_1_1
   glPolygonOffset(-2.0f, -1.0f);
   glEnable(GL_POLYGON_OFFSET_FILL);
#else
   glPolygonOffsetEXT(-0.1f, -0.002f);
   glEnable(GL_POLYGON_OFFSET_EXT);
#endif
   return 1;
}

static int PostDrawSubface(ssgEntity *)
{
#ifdef GL_VERSION_1_1
   glDisable(GL_POLYGON_OFFSET_FILL);
#else
   glDisable(GL_POLYGON_OFFSET_EXT);
#endif
   return 1;
}

/* convert a list of triangles to geosets using the pfdMesher
 * need a seperate geostate for each comb of tex/mtl/bind
 */
static ssgEntity *Build(fltState *state)
{
   /*const char t[9] = "plsTQSfFP";*/
   ssgBranch *grp = 0;
   ssgCutout *bboard1 = 0;
   ssgCutout *bboard2 = 0;
   ssgLeaf *leaf = 0;
   fltTriangle *arr;
   int num, i, j, k, m, n;

   //int *index;
   //int *vertex;

   arr = state->tris;
   num = state->ntris;
   state->ntris = 0;
   if (num == 0)
      return 0;

   /* sort the triangles (put them in bins according to their attributes)
    * one geoset will be generated for each such bin
    */
   qsort(arr, num, sizeof(fltTriangle), tricmp);

#ifdef USE_ALLOCA
   int *index = (int *)alloca(sizeof(index[0]) * state->vnum);
   int *vertex = (int *)alloca(sizeof(vertex[0]) * 32768);
#else
   int index[state->vnum];
   int vertex[32768];
#endif
   
   for (i = 0; i < num; ) {

      ssgVertexArray *va;
      ssgColourArray *ca = 0;
      ssgNormalArray *na = 0;
      ssgTexCoordArray *ta = 0;
      ssgVtxTable *geom;

      int flags = arr[i].flags;
      int bind = arr[i].bind;

      /* see how many tris that belong to this group */
      for (j = i + 1; (j < num && 
                       arr[j].state == arr[i].state && 
                       arr[j].flags == flags &&
		       arr[j].bind == bind
                       ); j++)
         ;
      /*fprintf(stderr, " %d", j - i);*/
      
      /* simply construct a triangle soup */
      /* XXX fixme! */

      if ((bind & BIND_FLAT) || (flags & TRI_COLOR_MATERIAL)) {

	 n = 3*(j - i);

	 va = new ssgVertexArray(n);
	 for (k = i; k < j; k++) {
	    va->add(state->coord[arr[k].index[0]]);
	    va->add(state->coord[arr[k].index[1]]);
	    va->add(state->coord[arr[k].index[2]]);
	 }
	 
	 if ((bind & BIND_FLAT_COLOR)) {
	    ca = new ssgColourArray(n);
	    for (k = i; k < j; k++) {
	       ca->add(arr[k].color);
	       ca->add(arr[k].color);
	       ca->add(arr[k].color);
	    }
	 }
	 else if ((bind & BIND_COLOR)) {
	    ca = new ssgColourArray(n);
	    if ((flags & TRI_COLOR_MATERIAL)) {
	       for (k = i; k < j; k++) {
		  for (m = 0; m < 3; m++) {
		     sgVec4 color;
		     float *mtl_color = arr[k].color;
		     float *vtx_color = state->color[arr[k].index[m]];
		     color[0] = mtl_color[0] * vtx_color[0];
		     color[1] = mtl_color[1] * vtx_color[1];
		     color[2] = mtl_color[2] * vtx_color[2];
		     color[3] = mtl_color[3] * vtx_color[3];
		     ca->add(color);
		  }
	       }
	    }
	    else {
	       for (k = i; k < j; k++) {
		  ca->add(state->color[arr[k].index[0]]);
		  ca->add(state->color[arr[k].index[1]]);
		  ca->add(state->color[arr[k].index[2]]);
	       }
	    }
	 }
	 else if ((flags & TRI_TRANSLUCENT)) {
	    ca = new ssgColourArray(1);
	    ca->add(arr[i].color);
	 }

	 if ((bind & BIND_FLAT_NORMAL)) {
	    na = new ssgNormalArray(n);
	    for (k = i; k < j; k++) {
	       sgVec3 a, b, normal;
	       int *w = arr[k].index;
	       sgSubVec3(a, state->coord[w[1]], state->coord[w[0]]);
	       sgSubVec3(b, state->coord[w[2]], state->coord[w[0]]);
	       sgVectorProductVec3(normal, a, b);
	       sgNormalizeVec3(normal);
	       na->add(normal);
	       na->add(normal);
	       na->add(normal);
	    }
	 }
	 else if ((bind & BIND_NORMAL)) {
	    na = new ssgNormalArray(n);
	    for (k = i; k < j; k++) {
	       na->add(state->normal[arr[k].index[0]]);
	       na->add(state->normal[arr[k].index[1]]);
	       na->add(state->normal[arr[k].index[2]]);
	    }
	 }

	 if ((bind & BIND_TEXCOORD)) {
	    ta = new ssgTexCoordArray(n);
	    for (k = i; k < j; k++) {
	       ta->add(state->texcoord[arr[k].index[0]]);
	       ta->add(state->texcoord[arr[k].index[1]]);
	       ta->add(state->texcoord[arr[k].index[2]]);
	    }
	 }

	 geom = new ssgVtxTable(GL_TRIANGLES, va, na, ta, ca);

      }
      else {

	 ssgIndexArray *ia = new ssgIndexArray(3*(j - i));
	 
	 memset(index, -1, sizeof(index[0]) * state->vnum);
	 n = 0;
	 for (k = i; k < j; k++) {
	    int *tri = arr[k].index;
	    int save = n;
	    for (m = 0; m < 3; m++) {
	       if (index[tri[m]] == -1) {
		  if (n == 65536) {
		     ulSetError(UL_DEBUG, "[flt] More than 65536 vertices, split forced.");
		     n = save;
		     j = k;
		     //goto hopp;
		     break;
		  }
		  vertex[n] = tri[m];
		  index[tri[m]] = n++;
	       }
	       ia->add((short)index[tri[m]]);
	    }
	    if (m < 3) break;
	 }
	 //hopp:
	 
	 va = new ssgVertexArray(n);
	 for (k = 0; k < n; k++)
	    va->add(state->coord[vertex[k]]);
	 
	 if ((bind & BIND_COLOR)) {
	    ca = new ssgColourArray(n);
	    for (k = 0; k < n; k++)
	       ca->add(state->color[vertex[k]]);
	 }
	 else if ((flags & TRI_TRANSLUCENT)) {
	    ca = new ssgColourArray(1);
	    ca->add(arr[i].color);
	 }
	 
	 if ((bind & BIND_NORMAL)) {
	    na = new ssgNormalArray(n);
	    for (k = 0; k < n; k++)
	       na->add(state->normal[vertex[k]]);
	 }
	 
	 if ((bind & BIND_TEXCOORD)) {
	    ta = new ssgTexCoordArray(n);
	    for (k = 0; k < n; k++)
	       ta->add(state->texcoord[vertex[k]]);
	 }

	 if (n == 3*(j - i)) {

	    delete ia;

	    geom = new ssgVtxTable(GL_TRIANGLES, va, na, ta, ca);
	 }
	 else {

	    geom = new ssgVtxArray(GL_TRIANGLES, va, na, ta, ca, ia);
	 }

      }

      geom->setState(arr[i].state);
      
      /*ulSetError(UL_DEBUG, "build: [%d..%d] %02x %d indices, %d vertices",
	     i, j - 1, bind,
	     3*(j - i), n);*/
      
      if (leaf) {
	 if (grp == 0)
	    grp = new ssgBranch;
	 grp->addKid(leaf);
      }

      leaf = geom;
      
#if 0
      if ((flags & TRI_TRANSLUCENT)) {
	 leaf->setCallback(SSG_CALLBACK_PREDRAW, PreDrawTranslucent);
      }
      else 
#endif
      if ((flags & TRI_SUBFACE)) {
	 leaf->setCallback(SSG_CALLBACK_PREDRAW, PreDrawSubface);
	 leaf->setCallback(SSG_CALLBACK_POSTDRAW, PostDrawSubface);
      }
      
#ifndef NO_LOADER_OPTIONS
      leaf = LoaderOptions->createLeaf(leaf, state->parent_name);
#endif
      
      if ((flags & TRI_BBOARD)) {
	 if ((flags & TRI_BBOARD_AXIAL)) {
	    if (bboard1 == 0)
	       bboard1 = new ssgCutout(0);
	    bboard1->addKid(leaf);	    
	 }
	 else {
	    if (bboard2 == 0)
	       bboard2 = new ssgCutout(1);
	    bboard2->addKid(leaf);
	 }
	 leaf = 0;
      }

      i = j;
   }

   if (((leaf != 0) + (bboard1 != 0) + (bboard2 != 0)) > 1 && !grp) //lint !e514 Lint warning "unusual use of a boolean"
      grp = new ssgBranch;

   if (grp) {
      if (bboard1) 
	 grp->addKid(bboard1);
      if (bboard2)
	 grp->addKid(bboard2);
      if (leaf) 
	 grp->addKid(leaf);
      return grp;
   }

   if (bboard1)
      return bboard1;

   if (bboard2)
      return bboard2;

   return leaf;
}

/* add a triangle */
static void AddTri(fltState *state, int v0, int v1, int v2)
{
   fltTriangle *tri;
   if (state->ntris == state->atris) {
      state->atris += state->atris;
      //state->tris = (fltTriangle *)realloc(state->tris, sizeof(fltTriangle) * state->atris);
      fltTriangle *old = state->tris;
      state->tris = new fltTriangle[state->atris];
      memcpy(state->tris, old, sizeof(fltTriangle) * state->atris / 2);
      delete [] old;
   }
   tri = state->tris + state->ntris++;
   memcpy(tri, state->temp, sizeof(fltTriangle));
   tri->index[0] = v0;
   tri->index[1] = v1;
   tri->index[2] = v2;
}

static void Triangulate(int *w, int n, fltState *state)
{
#ifdef USE_ALLOCA
   int *tris = (int *) alloca(sizeof(int) * 3 * (n - 2));
#else
   int tris[3 * (n - 2)];
#endif
   int num_tris = _ssgTriangulate(state->coord, w, n, tris);
   for (int i = 0; i < num_tris; i++)
      AddTri(state, tris[3*i + 0], tris[3*i + 1], tris[3*i + 2]);
}

static int ObsoleteFlag;
static int NotImplementedFlag;

static void Obsolete(int op)
{
   if (!ObsoleteFlag) {
      ulSetError(UL_WARNING, "[flt] This file is probably rather old (obsolete opcodes ignored).");
      ObsoleteFlag = 1;
   }
   //printf("op %d obsolete\n", op);
}

static void NotImplemented(int op)
{
   if (!NotImplementedFlag) {
      ulSetError(UL_WARNING, "[flt] This file contains opcodes that are not implemented.");
      NotImplementedFlag = 1;
   }
   //printf("op %d not implemented\n", op);
}

static void ReportBadChunk(const ubyte *ptr, const char *name)
{
   int op = get16u(ptr);
   int len = get16u(ptr + 2);
   ulSetError(UL_WARNING, "[flt] Bad record, opcode %d (%s), length %d:", op, name, len);
   hexdump(UL_WARNING, ptr, len, 0);
   ulSetError(UL_WARNING, "Please report this, either at http://plib.sourceforge.net/,");
   ulSetError(UL_WARNING, "or by email to plib-devel@lists.sourceforge.net. Thanks.");
}

#define BAD_CHUNK(ptr, name)\
   do {\
      static int first = 1;\
      if (first) {\
	 ReportBadChunk(ptr, name);\
	 first = 0;\
      }\
   } while (0)

static int intcmp(const void *a, const void *b)
{
   return *(int *)a - *(int *)b;
}

/* polygon chunks */
static int GeomChunks(ubyte *ptr0, ubyte *end, fltState *state, ssgEntity **nodep, int objflags, int objtrans)
{
   ubyte *ptr = ptr0;
   int sp = 0, op, len, done = 0, triflg = 0;
   int w[512]; /* enough? */
   fltTriangle tri;
   StateInfo info;
   int subface = 0;
   char long_id[256] = {0};

   state->temp = &tri;
   memset(&tri, 0, sizeof(tri));
   
   /* read as many polygon chunks as possible. 
    * can be done since objects may not contain groups.
    * color table etc should also be defined before any polygon.
    * (the purpose is to keep the switch(op) relatively small to speed up this "inner loop")
    */

   do {

      if (ptr + 4 > end)
         break;
      op = get16u(ptr);
      len = get16u(ptr+2);
      if (len < 4 || (len & 3) != 0 || ptr + len > end)
         break;
      
      switch (op) {
      
      case 5: { /* Face (appearence) */

         /*
          * Face Record Format           
          *
          * Int         0  2  Face Opcode 5 
          * Unsigned    2  2  Length of the record 
          * Char        4  8  7 char ASCII ID; 0 terminates 
          * Int        12  4  IR color code 
          * Int        16  2  Relative priority 
          * Int        18  1  Draw type 
          *                      0 = Draw solid with backface culling 
          *                      1 = Draw solid, no backface culling
          *                      2 = Draw wireframe 
          *                      3 = Draw wireframe and close 
          *                      4 = Surround with wireframe in alternate color 
          *                      8 = Omnidirectional light 
          *                      9 = Unidirectional light 
          *                      10 = Bidirectional light 
          * Int        19  1  Texture white = if TRUE, draw textured face white
          * Unsigned   20  2  Color name index 
          * Unsigned   22  2  Alternate color name index 
          * Int        24  1  Reserved 
          * Int        25  1  Template (billboard)
          *                      0 = Fixed, no alpha blending 
          *                      1 = Fixed, alpha blending 
          *                      2 = Axial rotate 
          *                      4 = Point rotate 
          * Int        26  2  Detail texture pattern index, -1 if none 
          * Int        28  2  Texture pattern index, -1 if none 
          * Int        30  2  Material index, -1 if none 
          * Int        32  2  Surface material code (for DFAD) 
          * Int        34  2  Feature ID (for DFAD)
          * Int        36  4  IR material code 
          * Unsigned   40  2  Transparency 0 = Opaque 65535 = Totally clear 
          * Unsigned   42  1  LOD generation control 
          * Unsigned   43  1  Line style index 
          * Boolean    44  4  Flags (bits from left to right) 
          *                      0 = Terrain 
          *                      1 = No color 
          *                      2 = No alternate color 
          *                      3 = Packed color 
          *                      4 = Terrain culture cutout (footprint) 
          *                      5 = Hidden, not drawn 
          *                      6-31 = Spare 
          * Unsigned   48  1  Light mode 
          *                      0 = Use face color, not illuminated 
          *                      1 = Use vertex colors, not illuminated 
          *                      2 = Use face color and vertex normal 
          *                      3 = Use vertex color and vertex normal 
          * Unsigned   49  1  Reserved 
          * Unsigned   50  2  Reserved 
          * Boolean    52  4  Reserved
          * Unsigned   56  4  Packed color, primary (A, B, G, R) 
          * Unsigned   60  4  Packed color, alternate (A, B, G, R)
          * Int        64  2  Texture mapping index 
          * Int        66  2  Reserved 
          * Unsigned   68  4  Primary color index 
          * Unsigned   72  4  Alternate color index 
          * Int        76  2  Reserved 
          * Int        78  2  Reserved 
          *
          */

	 static float default_mtl[14] = {
	    1.0f, 1.0f, 1.0f,
	    1.0f, 1.0f, 1.0f,
	    0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f,
	    8.0f, 1.0f
	 };
         int flags = len < 48 ? 0 : get32i(ptr + 44);
	 int drawtype = ptr[18];

         if (drawtype > 4 || (flags & (1 << 5)) || len < 32) {
            triflg = 0;
         }
         else {
            int index;
	    struct snode *t;
	    int tex_alpha = 0;

	    tri.state = 0;
	    tri.flags = 0;

#ifdef NO_TEXTURES
	    info.tex = 0;
#else
            /* texture */
	    info.tex = 0;
            index = get16i(ptr + 28); /* texture index */
	    t = 0;
	    if (index != -1) {
	       state->texs = t = splay(state->texs, (void *)index, ptrcmp);
	       if (t == 0 || t->key != (void *)index) {
		  /*ulSetError(UL_DEBUG, "[flt] undefined texture %d", index);*/
		  t = 0;
	       }
	    }
	    if (t) {
	       fltTexture *tex = (fltTexture *)t->data;
	       if (tex->state == (ssgState *)-1) {
		  fltTexture *tmp = LoadTex(tex->file);
		  tex->state = tmp->state;
		  tex->tex = tmp->tex;
		  tex->alpha = tmp->alpha;
	       }
	       tri.state = tex->state;
	       info.tex = tex->tex;
	       tex_alpha = tex->alpha;
	    }
#ifndef NO_LOADER_OPTIONS
	    else {
	       if (state->notex_state == (ssgState *)-1)
		  state->notex_state = LoaderOptions->createState(0);
	       tri.state = state->notex_state;
	    }
#endif
#endif

	    if (tri.state) {

	       tri.bind = BIND_COLOR | BIND_NORMAL | BIND_TEXCOORD;

	    }
	    else {
	    
	       int template = ptr[25];
	       int white = ptr[19] || template > 1; /* OK ?? */
	       int lmode = len >= 49 ? ptr[48] : 3;
	       int trans = get16u(ptr + 40);

	       //ulSetError(UL_DEBUG, "light mode %d", lmode);

	       /*lmode = 0;*/
	    
	       /* wireframe */
	       /*tri.wireframe = (drawtype > 1);*/

	       /* backface culling */
	       info.cf = (drawtype == 0);
	       
	       /* alpha component */
#if 0
	       if (state->major >= 15) {
		  /* this is according to the spec, but it sure is suspicious */
		  int a = trans >> 1;
		  int b = objtrans >> 1;
		  info.alpha = 1.0f - 1.0f/(32767*32767) * (a * b);
	       }
	       else 
#endif
               {
		  info.alpha = 1.0f - 1.0f/65535.0f * trans;
	       }
	       sgSetVec4(tri.color, 1, 1, 1, info.alpha);
	       
#ifdef NO_LIGHTING
	       info.mtl = 0;
#else
	       /* material */
	       if (/*lmode < 2 ||*/ /* (objflags & (1 << 3)) || */ white) {
		  info.mtl = 0;
	       }
	       else {
		  index = get16i(ptr + 30); /* material index */
		  state->mtls = t = splay(state->mtls, (void *)index, ptrcmp);
		  if (t && t->key == (void *)index) {
		     info.mtl = (float *)t->data;
		  }
		  else {
		     info.mtl = default_mtl;
		  }
		  info.alpha *= info.mtl[13];
		  tri.color[3] = 1.0f;
	       }
#endif

	       /* transparency */
	       info.tr = (tex_alpha > 0 || info.alpha < 0.999f); /* && template > 0 */
	       if (info.tr)
		  tri.flags |= TRI_TRANSLUCENT;

	       /* initial bindings */
	       tri.bind = 0;
	       if ((lmode & 1) && !white)
		  tri.bind |= BIND_COLOR;
	       if (info.mtl) {
		  tri.bind |= BIND_NORMAL;
		  if (!(lmode & 2) || (objflags & (1 << 4)))
		     tri.bind |= BIND_FLAT_NORMAL;
	       }
	       if (info.tex)
		  tri.bind |= BIND_TEXCOORD;

#ifdef NO_COLORS
	       tri.bind &= ~BIND_COLOR;
#else

	       /* face color */
	       if (!(lmode & 1) && !(flags & 2) && !white && state->major >= 14) { /* will probably not work with old files */
		  if ((flags & 8) && len >= 60) {
		     UNPACK_ABGR(tri.color, ptr + 56); /* packed ABGR */
		     tri.bind |= BIND_FLAT_COLOR;
		  }
		  else if (state->revision > 1400) {
		     int color;
		     if (state->revision > 1500 && len >= 72)
			color = get32i(ptr + 68);
		     else {
			color = get16u(ptr + 20);
			if (color == 65535)
			   color = -1;
		     }
		     int index = color / 128;
		     int intensity = color % 128;
		     if (color >= 0 && state->ctab && index < state->cnum) {
			UNPACK_ABGR2(tri.color, state->ctab[index], intensity);
			tri.bind |= BIND_FLAT_COLOR;		  
		     }
		     else if (color != -1)
			ulSetError(UL_DEBUG, "[flt] Bad face color %d.", color);
		  }
		  tri.color[3] = info.alpha;
	       }
#endif

	       /*
		 if (tri.bind & BIND_COLOR) 
		   ulSetError(UL_DEBUG, "use vertex colors");
		 if (tri.bind & BIND_FLAT_COLOR)
		   ulSetError(UL_DEBUG, "face color %.2f %.2f %.2f", 
			      tri.color[0], tri.color[1], tri.color[2]);
	        */

	       /* billboard */
	       if (template == 4)
		  tri.flags |= TRI_BBOARD_POINT;
	       else if (template == 2)
		  tri.flags |= TRI_BBOARD_AXIAL;

	       /* cannot create the state yet,
		* need to know the vertex bindings. */
	    }
             
            triflg = 1;
         }            
         ptr += len;
         break;
      }

      case 72: /* Vertex List (aka polygon) */
         if (triflg) {
            int i, n = (len - 4) >> 2;
            if (state->vtab && n > 2 && n < 512) { /* silently ignore if suspicious.. */
	       int bind = tri.bind;
               ubyte *p = ptr + 4;
               for (i = 0; i < n; ++i) {
		  /* could this be done any faster? */
		  /* it is tempting to reparse the vertex at the given offset */
                  int offset = get32i(p), *ptr;
		  ptr = (int *)bsearch(&offset, state->offset, state->vnum, 
				       sizeof(int), intcmp);
                  if (!ptr) {
		     ulSetError(UL_DEBUG, "[flt] Bad vertex offset %i.", offset);
		     break;
		  }
                  w[i] = ptr - state->offset;
		  bind &= state->bind[w[i]] | BIND_FLAT;
                  p += 4;
               }
               if (i == n) {

		  tri.bind = bind;

		  if (subface)
		     tri.flags |= TRI_SUBFACE;
		  else
		     tri.flags &= ~TRI_SUBFACE;

		  if (tri.state) {

		     Triangulate(w, n, state);

		  }
		  else {
		     /* finalize state */
		     
		     /* setup color material */
		     if ((bind & (BIND_COLOR | BIND_FLAT_COLOR)) &&
			 (bind & (BIND_NORMAL | BIND_FLAT_NORMAL))) {
			sgVec4 color;
			color[0] = 0.2f*info.mtl[0] + 0.8f*info.mtl[3];
			color[1] = 0.2f*info.mtl[1] + 0.8f*info.mtl[4];
			color[2] = 0.2f*info.mtl[2] + 0.8f*info.mtl[5];
			color[3] = info.alpha;
 			if ((tri.bind & BIND_FLAT_COLOR)) {
			   tri.color[0] *= color[0];
			   tri.color[1] *= color[1];
			   tri.color[2] *= color[2];
			   tri.color[3] *= color[3];
			}
			else {
			   sgCopyVec4(tri.color, color); /* apply to vertex colors */
			}
			info.cm = 1;
			tri.flags |= TRI_COLOR_MATERIAL;
		     }
		     else {
			info.cm = 0;
			tri.flags &= ~TRI_COLOR_MATERIAL;
		     }
		     
		     tri.state = ConstructState(&info);

		     Triangulate(w, n, state);

		     tri.state = 0;
		  }
               }
            }
         }
         ptr += len;
         break;

      case 89: /* Morph Vertex List */
         ptr += len;
         break;
	    
      case 10: /* Push */
         sp++;
         ptr += len;
         break;
	 
      case 11: /* Pop */
         if (sp == 0) {
            done = 1;
            break;
         }
         sp--;
         ptr += len;
         /*if (sp == 0)
	   done = 1;*/
         break;

      case 19: /* Push Subface */
	 subface++;
         ptr += len;
         break;

      case 20: /* Pop Subface */
	 subface--;
         ptr += len;
         break;

      case 33: /* Long ID */
         if (long_id[0] == 0) {
            int n = CLAMP(len - 4, 0, 255);
            memcpy(long_id, ptr + 4, n);
            long_id[n] = 0;
         }
         ptr += len;
         break;

      case 31: /* Text Comment */
      case 50: /* Vector (for light points) */
      case 21: /* Push Extension */
      case 22: /* Pop Extension */
      case 97: /* Line Style Record */
      case 122: /* Push Attribute */
      case 123: /* Pop Attribute */
         ptr += len;
         break;

      case 6: /* Vertex with ID (obsolete) */
      case 7: /* Short Vertex (obsolete) */
      case 8: /* Vertex with Color (obsolete) */
      case 9: /* Vertex with Color and Normal (obsolete) */
	 Obsolete(op);
	 ptr += len;
	 break;

      default:
         if (sp == 0) { /* you never know what this might be. better let someone else take car of it. */
            done = 1;
            break;
         }
	 NotImplemented(op);
         ptr += len;
      }

   } while (!done);
   
   *nodep = Build(state);

   if (long_id[0] != 0 && *nodep != NULL)
     (*nodep)->setName(long_id);

   return ptr - ptr0;
}

struct LODInfo {
   ssgTransform *scs;
   ssgRangeSelector *lod;   
};

static int LODCompare(const void *a, const void *b)
{
   float d = ((LODInfo *)a)->lod->getRange(0) - ((LODInfo *)b)->lod->getRange(0);
   return d < 0 ? -1 : d > 0 ? 1 : 0;
}

static void MergeLODs(ssgBranch *grp)
{
   LODInfo info[64];
   int i, j, k, m, n;

   n = grp->getNumKids();
   if (n <= 1)
      return;

   m = 0;   
   for (i = 0; i < n && m < 64; i++) {
      ssgEntity *kid;
      kid = grp->getKid(i);
      info[m].scs = 0;
      if (kid->isA(ssgTypeTransform()) && kid->getNumParents() == 1) {
	 info[m].scs = (ssgTransform *)kid;
	 if (info[m].scs->getNumKids() != 1)
	    continue;
	 kid = info[m].scs->getKid(0);
      }
      if (!kid->isA(ssgTypeRangeSelector()) || kid->getNumParents() != 1)
	 continue;
      info[m].lod = (ssgRangeSelector *)kid;
      m++;
   }
   
   if (m > 1) {
      
      qsort(info, m, sizeof(info[0]), LODCompare);

      //grp->print();
      
      for (i = 0; i < m; i++) {
	 sgMat4 mat1, mat2;	       
	 float ranges[33], range2;
	 int num1, num2;
	 int flag = 0;
	 
	 if (info[i].lod == 0)
	    continue;
	 
	 if (info[i].scs)
	    info[i].scs->getTransform(mat1);
	 else
	    sgMakeIdentMat4(mat1);
	 num1 = info[i].lod->getNumKids();
	 for (k = 0; k <= num1; k++)
	    ranges[k] = info[i].lod->getRange(k);
	 
	 for (j = i + 1; j < m; j++) {
	    
	    if (info[j].lod == 0)
	       continue;
	    
	    if (info[j].scs)
	       info[j].scs->getTransform(mat2);
	    else
	       sgMakeIdentMat4(mat2);
	    num2 = info[j].lod->getNumKids();
	    range2 = info[j].lod->getRange(0);
	    
	    if (sgDistanceVec3(mat1[3], mat2[3]) > 0.1f * range2) /* XXX */
	       continue;
	    if (num1 + num2 > 32) /* XXX */
	       continue;
	    if (ABS(ranges[num1] - range2) > 0.1f * range2) /* XXX */
	       continue;
	    
	    for (k = 0; k < num2; k++) {
	       info[i].lod->addKid(info[j].lod->getKid(k));
	       ranges[num1 + k + 1] = info[j].lod->getRange(k + 1);
	       num1++;
	    }
	    
	    if (info[j].scs == 0 || info[j].scs->getRef() == 1) {
	       if (info[j].lod->getRef() == 1) {
		  while (num2--)
		     info[j].lod->removeKid(num2);
	       }
	       if (info[j].scs)
		  info[j].scs->removeKid(info[j].lod); // delete info[j].lod
	       else
		  grp->removeKid(info[j].lod);
	    }
	    if (info[j].scs)
	       grp->removeKid(info[j].scs); // delete info[j].scs

	    info[j].lod = 0;	   
	    flag = 1;
	 }

	 if (flag) {
	    info[i].lod->setRanges(ranges, num1 + 1);
	    assert(info[i].scs == 0 || !info[i].scs->isA(0xDeadBeef));
	    assert(!info[i].lod->isA(0xDeadBeef));
	 }
      }
   }
}

static ssgEntity *PostClean(ssgEntity *node, fltNodeAttr *attr)
{

  if (node && attr && attr->name)
    node->setName(attr->name);

#if 1
   /* remove empty or redundant groups */
   while (!NoClean && node && node->isA(ssgTypeBranch())) {
      ssgBranch *grp = (ssgBranch *)node;
      int num = grp->getNumKids();
      if (num > 1)
	 break;
      if (num == 1) {
	 ssgEntity *kid = grp->getKid(0);
	 if (grp->getName()) {
	    if (kid->getName())
	       break;
	    kid->setName(grp->getName());
	 }
	 if (grp->getRef() == 0) {
	    kid->ref();
	    grp->removeKid(0);
	    kid->deRef();
	    delete grp;
	 }
	 node = kid;
	 assert(!node->isA(0xDeadBeef));
      }
      else {
	 if (grp->getRef() == 0)
	    delete grp;
	 node = 0;
      }
   }
#endif

#if 1
   /* see if we can merge LOD nodes */
   if (node && node->isAKindOf(ssgTypeBranch()) &&
       !node->isAKindOf(ssgTypeSelector())) {
      MergeLODs((ssgBranch *)node);
      assert(!node->isA(0xDeadBeef));
   }
#endif

   /* set limits on animated nodes */
   if (node && node->isA(ssgTypeTimedSelector())) {
      ssgTimedSelector *sw = (ssgTimedSelector *)node;
      if (sw->getNumKids() > 1) {
	 sw->setDuration(30.0f);
	 sw->setLimits(0, sw->getNumKids() - 1);			  
	 sw->control(SSG_ANIM_START);
      }
   }

   /* apply node attributes */
   if (node && attr) {

      if (attr->transform) {

	 ssgTransform *scs = new ssgTransform;	    
	 scs->setTransform(attr->mat);
	 scs->addKid(node);
	 
	 if (attr->replicate > 0) {
	    ssgBranch *grp;
	    sgMat4 mat;
	    int i;
	    grp = new ssgBranch;
	    grp->addKid(scs);
	    sgCopyMat4(mat, attr->mat);
#if 0
	    ulSetError(UL_DEBUG, "replicating %d times", attr->replicate);
	    for (i = 0; i < 4; i++)
	       ulSetError(UL_DEBUG, "  %6.2f %6.2f %6.2f %6.2f", mat[i][0], mat[i][1], mat[i][2], mat[i][3]);
#endif
	    for (i = 0; i < attr->replicate; ++i) {
	       sgPostMultMat4(mat, attr->mat);
	       scs = new ssgTransform;
	       scs->setTransform(mat);
	       scs->addKid(node);
	       grp->addKid(scs);
	    }
	    node = grp; /* there's no point in cleaning this construction */
	    /*ulSetError(UL_DEBUG, "[flt] replicated %d times", attr->replicate);*/
	 }
	 else {
	    node = scs; // PostClean(scs, 0);
	    /*ulSetError(UL_DEBUG, "[flt] added transformation");*/
	 }
      }
      
      /* apply level of detail */
      if (attr->islod) {
	 float ranges[2];
	 ranges[0] = MIN(attr->range[0], attr->range[1]);
	 ranges[1] = MAX(attr->range[0], attr->range[1]);
	 if (ranges[1] > MAX(0, ranges[0])) {
	    ssgRangeSelector *lod = new ssgRangeSelector;
	    lod->setRanges(ranges, 2);

	    if (sgLengthVec3(attr->center) > 0.01f) { /* XXX */
	       ssgTransform *t1, *t2;
	       sgMat4 mat;
	       sgMakeTransMat4(mat, attr->center[0], attr->center[1], attr->center[2]);
	       t1 = new ssgTransform;
	       t1->setTransform(mat);
	       sgMakeTransMat4(mat, -attr->center[0], -attr->center[1], -attr->center[2]);
	       t2 = new ssgTransform;
	       t2->setTransform(mat);
	       t1->addKid(lod);
	       lod->addKid(t2);
	       t2->addKid(node);
	       node = t1;
	    }
	    else {
	       lod->addKid(node);
	       node = lod;
	    }
	 }	 
      }
   }

   if (attr)
      delete attr;

   assert(node == 0 || !node->isA(0xDeadBeef));

   return node;
}

/* link nodes while removing crap (post traversal) */
static void PostLink(ssgEntity **stack, fltNodeAttr **attr)
{
#if 0
   ulSetError(UL_DEBUG, "PostLink: %s %s <-- %s %s",
           stack[0] ? pfGetTypeName(stack[0]) : "0", stack[0] ? pfGetNodeName(stack[0]) : "0",
           stack[1] ? pfGetTypeName(stack[1]) : "0", stack[1] ? pfGetNodeName(stack[1]) : "0");
#endif

   if (stack[1] == 0) {
      if (attr[1]) {
         delete attr[1];
	 attr[1] = 0;
      }
      return;
   }
   assert(!stack[1]->isA(0xDeadBeef));
   
   if (stack[0] == 0) {
      stack[0] = stack[1]; /* delay further processing */
      attr[0] = attr[1];
   }
   else {
      assert(!stack[0]->isA(0xDeadBeef));
      stack[1] = PostClean(stack[1], attr[1]);
      if (stack[1]) {
         if (stack[0]->isAKindOf(ssgTypeBranch())) {
	    ((ssgBranch *)stack[0])->addKid(stack[1]);
	 }
	 else {
            ssgBranch *grp = new ssgBranch;
            grp->addKid(stack[0]);
            grp->addKid(stack[1]);
            stack[0] = PostClean(grp, 0);
         }
      }
   }
   stack[1] = 0;
   attr[1] = 0;
}

/* group/object ancillary chunks */
static int AttrChunks(ubyte *ptr0, ubyte *end, fltNodeAttr **attrp)
{
   ubyte *ptr = ptr0;
   int op, len, done = 0;
   fltNodeAttr *attr;

   attr = *attrp;

   while (!done) {
      
      if (ptr + 4 > end)
         break;
      op = get16u(ptr);
      len = get16u(ptr + 2);
      if (len < 4 || (len & 3) != 0 || ptr + len > end)
         break;

      switch (op) {

      case 31: /* Comment */
         ptr += len;
         break;

      case 33: { /* Long ID */
	 int n = len - 4;
	 if (n > 0 && n < 256) {
	    if (attr == 0) 
	       attr = new fltNodeAttr;
	    //attr->name = (char *)malloc(n + 1);
	    attr->name = new char[n + 1];
	    memcpy(attr->name, ptr + 4, n);
	    attr->name[n] = 0;
	 }
         ptr += len;
         break;
      }

      case 60: { /* Replicate */
	 if (attr == 0)
	    attr = new fltNodeAttr;
	 attr->replicate = get16u(ptr + 4);
         ptr += len;
         break;
      }

      case 49: { /* Transfomation Matrix */
	 sgMat4 mat;
	 int i, j;
	 get32v(ptr + 4, mat, 16);
	 for (i = 0; i < 4; i++) {
	    for (j = 0; j < 4; j++) {
	       float d = mat[i][j] - (i == j ? 1.0f : 0.0f);
	       if (ABS(d) > 0.001f)
		  break;
	    }
	    if (j < 4)
	       break;
	 }
	 if (i < 4) {
	    if (attr == 0)
	       attr = new fltNodeAttr;
	    attr->transform = 1;
	    sgCopyMat4(attr->mat, mat);
	 }
         ptr += len;
         break;
      }

      case 12: /* Translate (obsolete) */
      case 40: /* Translate (obsolete) */
      case 41: /* Rotate about Point (obsolete) */
      case 42: /* Rotate about Edge (obsolete) */
      case 43: /* Scale (obsolete) */
      case 44: /* Translate (obsolete) */
      case 45: /* Scale (obsolete) */
      case 46: /* Rotate about Point (obsolete) */
      case 47: /* Rotate and/or Scale about Point (obsolete) */
      case 48: /* Put Transform (obsolete) */
      case 51: /* Bounding Box (obsolete) */
      case 77: /* Scale (obsolete) */
      case 110: /* Histogram Bounding Volume (obsolete) */
	 Obsolete(op);
	 ptr += len;
	 break;

      case 74: /* Bounding Box */
      case 76: /* Rotate about Edge */
      case 78: /* Translate */
      case 79: /* Scale */
      case 80: /* Rotate about Point */
      case 81: /* Rotate and/or Scale */
      case 82: /* Put */
      case 88: /* Road Zone */
      case 94: /* General Matrix */
      case 100: /* Extension Attribute */
      case 105: /* Bounding Sphere */
      case 106: /* Bounding Cylinder */
      case 108: /* Bounding Volume Center */
      case 109: /* Bounding Volume Orientation */
      case 116: /* CAT Data */
	 /* these are safe to ignore */
         ptr += len;
         break;

      default:
         done = 1;
         break;
      }
   }

   *attrp = attr;

   return ptr - ptr0;
}

static ssgEntity *LoadFLT(const char *path);

/* object hierarchy chunks */
static ssgEntity *HierChunks(ubyte *ptr, ubyte *end, fltState *state)
{
   ssgEntity *stack[MAXDEPTH + 1];
   fltNodeAttr *attr[MAXDEPTH + 1];
   int sp, op, len, k;

   stack[0] = new ssgBranch;
   stack[0]->setName("reserved");
   attr[0] = 0;
   stack[1] = 0;
   attr[1] = 0;
   sp = 1;

   for (;;) {

      if (ptr + 4 > end)
         break;
      op = get16u(ptr);
      if (op < 1 || op > 150)
         break;
      len = get16u(ptr + 2);
      if (len < 4 || (len & 3) != 0 || ptr + len > end)
         break;
      
      switch (op) {

      case 10: /* Push */
         if (stack[sp] && !stack[sp]->isAKindOf(ssgTypeBranch())) {
            /* shouldn't happen */
            ulSetError(UL_DEBUG, "[flt] Objects are not allowed to contain other objects or groups.");
            PostLink(stack + sp - 1, attr + sp - 1);
         }
	 if (sp >= MAXDEPTH) {
	    ulSetError(UL_WARNING, "[flt] Stack overflow.");
	 }
	 else {
	    sp++;
	    stack[sp] = 0;
	    attr[sp] = 0;
	 }
	 ptr += len;
         break;
         
      case 11: /* Pop */
         if (sp == 1) 
            ulSetError(UL_WARNING, "[flt] Stack underflow.");
         else {
            PostLink(stack + sp - 1, attr + sp - 1);
            sp--;            
         }
         ptr += len;
         break;

      case 4: { /* Object */

         /*
          * Object Record Format 
          *
          * Int        0  2  Object Opcode 4 
          * Unsigned   2  2  Length of the record 
          * Char       4  8  7 char ASCII ID; 0 terminates 
          * Boolean   12  4  Flags (bits from to right) 
          *                     0 = Don't display in daylight 
          *                     1 = Don't display at dusk 
          *                     2 = Don't display at night 
          *                     3 = Don't illuminate 
          *                     4 = Flat shaded 
          *                     5 = Group's shadow object 
          *                     6-31 = Spare 
          * Int       16  2  Relative priority 
          * Unsigned  18  2  Transparency
          *                     0 = Opaque 
          *                     65535 = Totally clear 
          * Int       20  2  Special effect ID1 - application defined 
          * Int       22  2  Special effect ID2 - application defined 
          * Int       24  2  Significance 
          * Int       26  2  Spare
          *
          */

	 int flags = get32i(ptr + 12);
	 int trans = get16u(ptr + 18);
         PostLink(stack + sp - 1, attr + sp - 1);
         ptr += len;
         ptr += AttrChunks(ptr, end, &attr[sp]);
	 state->parent_name = (char *)ptr + 4; /* OK?? */
         ptr += GeomChunks(ptr, end, state, &stack[sp], flags, trans);
	 state->parent_name = 0;
	 if (stack[sp] && stack[sp]->getName() == 0 && ptr[4])
	    stack[sp]->setName((char *)ptr + 4);
         break;
      }

      case 5: /* Face */
         /* polygons are not allowed to be outside objects, but this rule is not always respected */
	 //ulSetError(UL_DEBUG, "[flt] Implicit object.");
         PostLink(stack + sp - 1, attr + sp - 1);
         ptr += GeomChunks(ptr, end, state, &stack[sp], 0, 0);
         break;

      case 2: /* Group */
      case 14: /* Degree of Freedom */
      case 73: /* Level of Detail */
      case 96: /* Switch */
      case 98: /* Clip Region */

         PostLink(stack + sp - 1, attr + sp - 1);

	 switch (op) {
	 case 14: 
	    stack[sp] = new ssgTransform; 
	    break;
#if 1
	 case 96: {
	    ssgTimedSelector *sw = new ssgTimedSelector; 
	    stack[sp] = sw;
	    break;
	 }
#endif
	 case 73: {
	    fltNodeAttr *a;
	    double v[3];
	    attr[sp] = a = new fltNodeAttr;
	    a->islod = 1;
	    get64v(ptr + 16, v, 2);
	    sgSetVec2(a->range, (float)v[0], (float)v[1]);
	    get64v(ptr + 40, v, 3);
	    sgSetVec3(a->center, (float)v[0], (float)v[1], (float)v[2]);
            stack[sp] = new ssgBranch;
	    break;
	 }
	 default: //lint !e616
            stack[sp] = new ssgBranch;
	    break;
         }
	 if (ptr[4])
	    stack[sp]->setName((char *)ptr + 4);
         ptr += len;
         ptr += AttrChunks(ptr, end, &attr[sp]);
         break;

      case 61: /* Instance Reference */
         k = get16u(ptr + 6);
         /*ulSetError(UL_DEBUG, "[flt] instance reference %d", k);*/
         PostLink(stack + sp - 1, attr + sp - 1);	 
         if (state->refs) {
	    state->refs = splay(state->refs, (void *)k, ptrcmp);
	    if (state->refs->key == (void *)k)
	       stack[sp] = (ssgEntity *)state->refs->data;
	 }
         ptr += len;
         ptr += AttrChunks(ptr, end, &attr[sp]);
         break;

      case 62: /* Instance Definition */
         k = get16u(ptr + 6);
         /*ulSetError(UL_DEBUG, "[flt] instance definition %d", k);*/
         if (stack[sp]) {
	    state->refs = sinsert(state->refs, (void *)k, 0, ptrcmp);
	    if (state->refs->data == (void *)-1) {
	       state->refs->data = stack[sp];
	       stack[sp]->ref();
	    }
	 }
         ptr += len;
         break;

      case 63: /* External Reference */
         /*ulSetError(UL_DEBUG, "external reference %s", ptr + 4);*/
         PostLink(stack + sp - 1, attr + sp - 1);
         if (!NoExternals) {
            char *file = (char *)ptr + 4, *p;
	    if ((p = strrchr(file, '/')))
		file = p + 1;
	    //stack[sp] = LoadFLT(file);
	    stack[sp] = ssgLoad (file);
         }
         ptr += len;
         ptr += AttrChunks(ptr, end, &attr[sp]);
         break;

      case 31: /* Text Comment */
      case 21: /* Push Extension */
      case 22: /* Pop Extension */
      case 55: /* Binary Separating Plane */
      case 87: /* Road Segment */
      case 90: /* Linkage */
      case 91: /* Sound Bead */
      case 92: /* Path Bead */
      case 95: /* Text */
      case 101: /* Light Source Bead */
      case 122: /* Push Attribute */
      case 123: /* Pop Attribute */
	//printf("op %d ignored\n", op);
	 ptr += len;
	 break;

      case 3:  /* Level of Detail (obsolete) */
      case 13: /* Degree of Freedom (obsolete) */
      case 16: /* Instance Reference (obsolete) */
      case 17: /* Instance Definition (obsolete) */
	 Obsolete(op);
         ptr += len;
         break;

      default:
	 NotImplemented(op);
         ptr += len;
      }
   }

   /* pop stack (expected one iteration but may be more for incomplete databases) */
   while (sp-- > 0)
      PostLink(stack + sp, attr + sp);
   
   if (stack[0])
      stack[0] = PostClean(stack[0], attr[0]);
   else if (attr[0])
      delete attr[0]; // possible?

   return stack[0];
}

/* parse the vertex table */
static int VertexTable(ubyte *ptr0, ubyte *end, fltState *state)
{
   int len, size, num, i;
   ubyte *ptr = ptr0;

   assert(get16u(ptr) == 67); /* vtx tab op code */
   len = get16u(ptr + 2);

   size = get32i(ptr + 4);
   /*ulSetError(UL_DEBUG, "vertex table header len %d, total size %d", len, size);*/
   num = (size - len)/40; /* max # of vertices */
   if (num <= 0 || state->vtab) {
      if (state->vtab)
	 ulSetError(UL_WARNING, "[flt] Multiple vertex tables not allowed.");
      return size;
   }
   state->vtab = ptr;
   end = MIN(end, ptr + size);
   ptr += len;

   state->offset   = new int [num];
   state->bind     = new ubyte [num];
   state->coord    = new sgVec3 [num];
   state->color    = new sgVec4 [num];
   state->normal   = new sgVec3 [num];
   state->texcoord = new sgVec2 [num];

   for (i = 0; i < num && ptr + 40 <= end; ++i) {
      double tmp[3];
      ubyte *p = ptr;
      int op = get16u(p);
      int len = get16u(p + 2);
      int flags = get16u(p + 6);
      int bind = 0;
      if (ptr + len > end)
         break;
      state->offset[i] = ptr - ptr0;
      get64v(p + 8, tmp, 3);
      sgSetVec3(state->coord[i], (float)tmp[0], (float)tmp[1], (float)tmp[2]);
      sgSetVec4(state->color[i], 1, 1, 1, 1);
      sgSetVec3(state->normal[i], 0, 0, 1);
      sgSetVec2(state->texcoord[i], 0, 0);
      p += 32;
      if (op == 69 || op == 70) {
	 get32v(p, state->normal[i], 3);
	 sgNormalizeVec3(state->normal[i]);
	 p += 12;
	 bind |= BIND_NORMAL;
      }
      if (op == 70 || op == 71) {
         get32v(p, state->texcoord[i], 2);
         p += 8;
         bind |= BIND_TEXCOORD;
      }
      if (!(flags & 4)) {
	 if ((flags & 8) && p + 4 <= ptr + len) {
	    UNPACK_ABGR(state->color[i], p);
	    //ulSetError(UL_DEBUG, "packed ABGR: %d %d %d %d", p[0], p[1], p[2], p[3]);
	    bind |= BIND_COLOR;
	 }
	 else if (state->revision > 1400) {
	    int color;
	    if (state->revision > 1500 && p + 8 <= ptr + len) {
	       color = get32i(p + 4);
	    }
	    else {
	       color = get16u(ptr + 4);
	       if (color == 65535)
		  color = -1;
	    }
	    int index = color / 128;
	    int intensity = color % 128;
	    if (color >= 0 && state->ctab && index < state->cnum) {
	       //ulSetError(UL_DEBUG, "colour %d: index %d, intensity %d", color, index, intensity);
	       UNPACK_ABGR2(state->color[i], state->ctab[index], intensity);
	       bind |= BIND_COLOR;
	    }
	 }
      }
      /*
       if (bind & BIND_COLOR)
	 ulSetError(UL_DEBUG, "vertex #%d color %.2f %.2f %.2f", i,
		state->color[i][0],
		state->color[i][1],
		state->color[i][2]);
      */
      state->bind[i] = bind;
      ptr += len;      
   }
   /*ulSetError(UL_DEBUG, "got %d vertices", i);*/

   state->vnum = i;

   return size;
}

/* header ancillary chunks */
static int TableChunks(ubyte *ptr0, ubyte *end, fltState *state)
{
   ubyte *ptr = ptr0;
   int op, len, index, done = 0;

   while (!done) {
      
      if (ptr + 4 > end)
         break;
      op = get16u(ptr);
      len = get16u(ptr + 2);
      if (len < 4 || (len & 3) != 0 || ptr + len > end)
         break;

      switch (op) {

      case 32: /* Color Table */
	 if (len < 132 + 4 * 512) {
	    /* note: in older files it appears that the
	     * color table is stored as 16-bit integers.
	     * i have no documentation on this.
	     * maybe 16 x 16 colors (rather than 512 or 1024 x 128).
	     */
	    if (state->revision <= 1400) {
	       ulSetError(UL_WARNING, "[flt] Color table ignored (unknown format).");
	       /*hexdump(UL_DEBUG, ptr, len, 0);*/
	    }
	    else
	       BAD_CHUNK(ptr, "Color Table");
	 }
	 else  if (state->ctab == 0) {
	    int max = (len - 132) / 4;
            state->ctab = (ubyte (*)[4])(ptr + 132);
	    state->cnum = MIN(state->revision > 1500 ? 1024 : 512, max);
#if 0
	    int i;
	    for (i = 0; i < state->cnum; i++)
	       ulSetError(UL_DEBUG, "%d %02x %02x %02x %02x", i, 
		                    state->ctab[i][0], state->ctab[i][1], 
		                    state->ctab[i][2], state->ctab[i][3]);
#endif
	 }
	 else {
	    ulSetError(UL_WARNING, "[flt] Multiple color tables are not allowed.");
	 }
         ptr += len;
         break;

      case 64: /* Texture Reference */
         if (!NoTextures) {
	    if (len == 96 || len == 216) {
	       char *file = (char *)ptr + 4, *p;
	       if ((p = strrchr(file, '/')))
		  file = p + 1;
	       index = get32i(ptr + len - 12);
	       state->texs = sinsert(state->texs, (void *)index, 0, ptrcmp);
	       if (state->texs->data == (void *)-1) {
		  //fltTexture *tex = (fltTexture *)malloc(sizeof(fltTexture));
		  fltTexture *tex = new fltTexture;
		  assert ( tex != NULL );
		  tex->file = file;
		  tex->state = (ssgState *)-1;
		  tex->tex = (ssgTexture *)-1;
		  state->texs->data = tex;
	       }
	    }
	    else {
	       BAD_CHUNK(ptr, "Texture Reference");
	    }
	 }
         ptr += len;
         break;

      case 66: /* Material Table */
	 if ((len - 4) % 184 != 0) {
	    BAD_CHUNK(ptr, "Material Table");
	 }
	 else if (state->mtls == 0) {
	    ubyte *p = ptr + 4;
            int i, j, n = (len - 4) / 184;
            for (i = 0; i < n; ++i) {
	       float *mtl;
	       state->mtls = sinsert(state->mtls, (void *)i, 0, ptrcmp);
	       //state->mtls->data = malloc(sizeof(float)*14);
	       state->mtls->data = new float[14];
	       mtl = (float *)state->mtls->data;
               get32v(p, mtl, 14);
	       for (j = 0; j < 12; j++)
		  mtl[j] = CLAMP(mtl[j], 0, 1);
	       mtl[12] = CLAMP(mtl[12], 8, 128);
	       mtl[13] = CLAMP(mtl[13], 0, 1);
#if 0
	       float r, g, b, s;
	       r = mtl[0] + mtl[3] + mtl[6] + mtl[9];
	       g = mtl[1] + mtl[4] + mtl[7] + mtl[10];
	       b = mtl[2] + mtl[5] + mtl[8] + mtl[11];
	       s = MAX3(r, g, b);
	       if (s > 1.0f) {
		  s = 1.0f/s;
		  for (j = 0; j < 12; j++)
		     mtl[j] *= s;
	       }
#endif
               p += 184;
            }
         }
	 else {
	    ulSetError(UL_WARNING, "[flt] Multiple material tables are not allowed");
	 }
	 ptr += len;
         break;

      case 67: /* Vertex Table (header) */
         ptr += VertexTable(ptr, end, state);
         break;

      case 113: /* Material */
	 index = get32i(ptr + 4);
	 state->mtls = sinsert(state->mtls, (void *)index, 0, ptrcmp);
	 if (state->mtls->data == (void *)-1) {
	    float *mtl;
	    int i;
	    //state->mtls->data = malloc(sizeof(float)*14);
	    state->mtls->data = new float[14];
	    mtl = (float *)state->mtls->data;
	    get32v(ptr + 24, mtl, 14);
	    for (i = 0; i < 12; i++)
	       mtl[i] = CLAMP(mtl[i], 0, 1);
	    mtl[12] = CLAMP(mtl[12], 8, 128);
	    mtl[13] = CLAMP(mtl[13], 0, 1);
#if 0
	    float r, g, b, s;
	    r = mtl[0] + mtl[3] + mtl[6] + mtl[9];
	    g = mtl[1] + mtl[4] + mtl[7] + mtl[10];
	    b = mtl[2] + mtl[5] + mtl[8] + mtl[11];
	    s = MAX3(r, g, b);
	    if (s > 1.0f) {
	       s = 1.0f/s;
	       for (i = 0; i < 12; i++)
		  mtl[i] *= s;
	    }
#endif
	 }
         ptr += len;
         break;         

      case 65: /* Eyepoint Palette (obsolete) */
	 Obsolete(op);
	 ptr += len;
	 break;

      case 31: /* Text Comment */
      case 83: /* Eyepoint and Trackplane */
      case 90: /* Linkage Palette */
      case 93: /* Sound Palette */
      case 97: /* Line Style Palette */
      case 102: /* Light Source Palette */
      case 103: /* Reserved */
      case 104: /* Reserved */
      case 112: /* Texture Mapping */
      case 114: /* Name Table */
	 /* these are safe to ignore */
	 ptr += len;
	 break;

      default:
	//printf("op %d: end of table chunks\n", op);
         done = 1;
      }
   }

   return ptr - ptr0;
}

static int CheckHeader(ubyte *ptr, ubyte *end, fltState *state)
{
   /*const char *unit[9] = {"meters", "kilometers", "", "", "feet", "inches", "", "", "nautical miles"};*/
   int len, k;
   if (get16u(ptr) != 1) {
      ulSetError(UL_WARNING, "[flt] Wrong header opcode (%d).", get16i(ptr));
      return -1;
   }
   len = get16u(ptr + 2);
   if (len < 128 || len > 1024) {      
      ulSetError(UL_WARNING, "[flt] Suspicious header record length (%d).", len);
      return -1;
   }
   k = get32i(ptr + 12);
   if (k < 100) {
      state->revision = 100 * k;
      state->major = k;
      state->minor = 0;
   }
   else {
      state->revision = k;
      state->major = k / 100;
      state->minor = k % 100;
   }
   if (state->major < 11 || state->major > 16) {
      ulSetError(UL_WARNING, "[flt] Suspicious format revision number (%d).", k);
      return -1;
   }
/*
   ulSetError(UL_DEBUG, "[flt] Loading %s %sFlight v%d.%d",
	   state->filename,
	   state->major > 13 ? "Open" : "",
	   state->major,
	   state->minor);
*/
   return len;
}

#if 0
struct snode *refs;
static int nrefs;

static void ptree(ssgEntity *node, FILE *f, int d)
{
   int i;   
   for (i = 0; i < d; ++i)
      fprintf(f, "   ");
   {
      const char *p = node->getName();
      fprintf(f, "%s \"%s\"", node->getTypeName(), p ? p : "");
   }
   if (node->getNumParents() > 1) {
      refs = sinsert(refs, node, 0, ptrcmp);
      if (refs->data == (void *)-1) {
         fprintf(f, " (%d)", nrefs);
	 refs->data = (void *)nrefs++;
      }
      else {
         fprintf(f, " (%d) ...\n", (int)refs->data);
         return;
      }
   }
   assert(!node->isA(0xDeadBeef));
   if (node->isAKindOf(ssgTypeLeaf())) {
      ssgLeaf *leaf = (ssgLeaf *)node;
      ssgState *st = leaf->getState();
      fprintf(f, " %d tris", leaf->getNumTriangles());
      if (st && st->isAKindOf(ssgTypeSimpleState())) {
	 ssgSimpleState *ss = (ssgSimpleState *)st;
	 char *file = ss->getTextureFilename();
	 if (file) {
	    char *p;
	    if ((p = strrchr(file, '/'))) 
	       file = p + 1;
	    fprintf(f, " %s", file);
	 }
      }
   }
   if (node->isAKindOf(ssgTypeTransform())) {
      sgMat4 mat;
      ((ssgTransform *)node)->getTransform(mat);
      fprintf(f, " pos %.2f %.2f %.2f", mat[3][0], mat[3][1], mat[3][2]);
   }
   if (node->isAKindOf(ssgTypeRangeSelector())) {
      ssgRangeSelector *lod = (ssgRangeSelector *)node;
      fprintf(f, " ranges");
      for (i = 0; i <= lod->getNumKids(); i++)
	 fprintf(f, " %.2f", lod->getRange(i));
   }
   putc('\n', f);
   if (node->isAKindOf(ssgTypeBranch())) {
      ssgBranch *grp = (ssgBranch *)node;
      int n = grp->getNumKids();
      for (i = 0; i < n; ++i)
	 ptree(grp->getKid(i), f, d+1);
   }
}
#endif

static const char *FindFile(const char *file)
{
   /* XXX fixme! */
   static char path[1024];

   if (ulFileExists((char *)file))
      return file;

   ssgGetCurrentOptions () -> makeModelPath ( path, file ) ;
   if (ulFileExists(path))
      return path;
   
   ulSetError(UL_WARNING, "[flt] %s not found.", file);
   
   return 0;
}

static struct snode *FltCache;

#define win32_perror(s) perror(s) /* no idea if this is appropriate.. */

static ssgEntity *LoadFLT(const char *file)
{
   struct snode *cache;
   cache = FltCache = sinsert(FltCache, (void *)file, strlen(file) + 1, (sfunc)strcmp);
   if (cache->data == (void *)-1) {
      cache->data = NULL; /* avoid looping */
      
      fltState *state = 0;
      int size = 0, len;
      ubyte *ptr = 0, *end;
      ssgEntity *node = 0;

#ifdef USE_WIN32_MMAP

      HANDLE fd = INVALID_HANDLE_VALUE;
      HANDLE map = 0;

#else

      int fd = -1;
#ifdef USE_POSIX_MMAP
      struct stat st;
#else
      ubyte buf[256];
#endif

#endif
      
      do { /* dummy loop */

	 const char *path, *name;

	 path = FindFile(file);
	 if (path == 0)
	    break;

#ifdef USE_WIN32_MMAP
	 
	 fd = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
			 FILE_ATTRIBUTE_NORMAL, 0);
	 if (fd == INVALID_HANDLE_VALUE) {
	    win32_perror(path);
	    break;
	 }
	 
	 size = GetFileSize(fd, NULL);
	 if (size < 256)
	    break;

	 map = CreateFileMapping(fd, NULL, PAGE_READONLY, 0, size, NULL);
	 if (map == 0) {
	    win32_perror(path);
	    break;
	 }

	 ptr = (ubyte *)MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
	 if (ptr == 0) {
	    win32_perror(path);
	    break;
	 }

#else /* USE_WIN32_MAP */

	 if ((fd = open(path, O_RDONLY|O_BINARY)) == -1) {
	    perror(path);
	    break;
	 }

#ifdef USE_POSIX_MMAP
	 if (fstat(fd, &st)) {
	    perror(file);
	    break;
	 }
	 size = st.st_size;
	 if (size < 256)
	    break;
	 ptr = (ubyte *)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
	 if (ptr == (ubyte *)-1) {
	    perror(file);
	    break;
	 }
	 close(fd);
	 fd = -1;
#else /* USE_POSIX_MMAP */
	 if (read(fd, buf, 256) != 256) {
	    perror(file);
	    break;
	 }
	 ptr = buf;
#endif /* ! USE_POSIX_MMAP */

#endif /* ! USE_WIN32_MMAP */
	 
	 /*putc('\n', stderr);*/

	 name = strrchr(file, '/');
	 if (name)
	    name++;
	 else
	    name = file;

	 state = new fltState;
	 state->filename = name;
	 
	 len = CheckHeader(ptr, ptr + 256, state);
	 if (len == -1)
	    break;
	 
#if !defined(USE_POSIX_MMAP) && !defined(USE_WIN32_MMAP)

	 if ((size = lseek(fd, 0, SEEK_END)) == -1) {
	    perror(file);
	    break;
	 }
	 //ptr = (ubyte *)malloc(size);
	 ptr = new ubyte[size];
	 lseek(fd, 0, SEEK_SET);
	 if (read(fd, ptr, size) != size) {
	    perror(file);
	    break;
	 }
	 close(fd);
	 fd = -1;

#endif
	 
	 end = ptr + size;
	 len += TableChunks(ptr + len, end, state);
	 node = HierChunks(ptr + len, end, state);
	 
	 if (node)
	    node->setName((char *)name);
	 
      } while (0);

      delete state;

#ifdef USE_WIN32_MMAP
      
      if (ptr) 
	 UnmapViewOfFile(ptr);
      if (map)
	 CloseHandle(map);
      if (fd != INVALID_HANDLE_VALUE)
	 CloseHandle(fd);

#else /* USE_WIN32_MMAP */
      
      if (fd != -1)
	 close(fd);
      
#ifdef USE_POSIX_MMAP
      if (ptr != 0 && ptr != (ubyte *)-1)
	 munmap((char *)ptr, size);
#else
      if (ptr != 0 && ptr != buf)
	 //free(ptr);
	 delete [] ptr;
#endif

#endif /* ! USE_WIN32_MMAP */
      
      cache->data = node;
      if (node) node->ref();
   }

   return (ssgEntity *)cache->data;
}

#if 0
static struct snode *Flattened;

static ssgEntity *Flatten(ssgEntity *node, float (*mat)[4])
{
   if (node == 0)
      return 0;

   assert(!node->isA(0xDeadBeef));

   /* some nodes cannot be flattened. only types that may be 
    * generated by the loader are considered here.
    */
   if (node->getRef() > 0 || 
       node->isAKindOf(ssgTypeRangeSelector()) ||
       node->isAKindOf(ssgTypeCutout())) {
      if (node->isAKindOf(ssgTypeBranch())) {
	 /* see if we have flattened beyond this node already */
	 int completed = 0;
	 if (node->getNumParents() > 0) {
	    Flattened = sinsert(Flattened, node, 0, ptrcmp);
	    if (Flattened->data == (void *)-1) {
	       Flattened->data = 0;
	    }
	    else {
	       completed = 1;
	    }
	 }
	 if (!completed) {
	    /* traverse */
	    ssgBranch *grp = (ssgBranch *)node;
	    int i, n = grp->getNumKids();
	    if (n > 0) {
#ifdef USE_ALLOCA
	       ssgEntity **kids = (ssgEntity **)alloca(sizeof(ssgEntity *) * n);
#else
	       ssgEntity *kids[n];
#endif
	       for (i = n; i--;) {
		  kids[i] = grp->getKid(i);
		  kids[i]->ref(); grp->removeKid(i); kids[i]->deRef();
		  kids[i] = Flatten(kids[i], 0);
		  assert(kids[i] == 0 || !kids[i]->isA(0xDeadBeef));
	       }
	       for (i = 0; i < n; i++)
		  if (kids[i])
		     grp->addKid(kids[i]);
	       grp->recalcBSphere();
	    }
	 }
      }
      /* insert a transform if required */
      if (mat) {
	 ssgTransform *scs = new ssgTransform;
	 scs->setTransform(mat);
	 scs->addKid(node);
	 node = scs;
      }
      return node;
   }

   assert(node->getNumParents() == 0);

   /* accumulate transformations */
   if (node->isAKindOf(ssgTypeTransform())) {
      ssgTransform *scs = (ssgTransform *)node;
      int i, n = scs->getNumKids();
      if (n <= 0) {
	 delete scs;
	 return 0;
      }
      else {
	 sgMat4 tmp;
	 scs->getTransform(tmp);
	 if (mat)
	    sgPostMultMat4(tmp, mat);
	 if (n == 1) {
	    node = scs->getKid(0);
	    node->ref(); scs->removeKid(0); node->deRef();
	    delete scs;
	    assert(!node->isA(0xDeadBeef));
	    return Flatten(node, tmp);
	 }
	 else {
	    ssgBranch *grp = new ssgBranch;
	    grp->setName(scs->getName());
	    for (i = 0; i < n; i++)
	       grp->addKid(scs->getKid(i));
	    while (n--)
	       scs->removeKid(n);
	    delete scs;
	    return Flatten(grp, tmp);
	 }
      }
   }

   /* recurse the scene graph */
   if (node->isAKindOf(ssgTypeBranch())) {
      ssgBranch *grp = (ssgBranch *)node;
      int i, n = grp->getNumKids();
      if (n <= 0) {
	 delete grp;
	 return 0;
      }
      else if (n == 1 && grp->isA(ssgTypeBranch())) {
	 ssgEntity *kid = grp->getKid(0);
	 kid->ref(); grp->removeKid(0); kid->deRef();
	 delete grp;
	 assert(!kid->isA(0xDeadBeef));
	 return Flatten(kid, mat);
      }
      else {
#ifdef USE_ALLOCA
	 ssgEntity **kids = (ssgEntity **)alloca(sizeof(ssgEntity *) * n);
#else
	 ssgEntity *kids[n];
#endif
	 for (i = n; i--;) {
	    kids[i] = grp->getKid(i);
	    kids[i]->ref(); grp->removeKid(i); kids[i]->deRef();
	    kids[i] = Flatten(kids[i], mat);
	    assert(kids[i] == 0 || !kids[i]->isA(0xDeadBeef));
	 }
	 assert(!grp->isA(0xDeadBeef));
	 for (i = 0; i < n; i++) {
	    if (kids[i]) {
	       assert(!kids[i]->isA(0xDeadBeef));
	       grp->addKid(kids[i]);
	    }
	 }
	 grp->recalcBSphere();
	 return grp;
      }
   }

   /* transform geometry */
   if (node->isAKindOf(ssgTypeVtxTable()) && mat) {
      ssgVtxTable *g = (ssgVtxTable *)node;
      sgVec3 *a;
      int i, n;
      n = g->getNumVertices();
      if (n > 0) {
	 g->getVertexList((void **)&a);
	 for (i = 0; i < n; i++)
	    sgXformPnt3(a[i], a[i], mat);
      }
      n = g->getNumNormals();
      if (n > 0) {
	 g->getNormalList((void **)&a);
	 for (i = 0; i < n; i++)
	    sgXformVec3(a[i], a[i], mat);
      }
      g->recalcBSphere();
   }
   assert(!node->isA(0xDeadBeef));
   return node;
}
#endif

static void Init(void)
{
   if (Inited)
      return;
   Inited = 1;

#if !defined(USE_POSIX_MMAP) && !defined(USE_WIN32_MMAP)
   ulSetError(UL_DEBUG, "[flt] Memory mapped files not enabled, please check source code.");
#endif

#ifdef PROBE_ENDIAN
   {
      short tmp = 42;
      if (*(char *)&tmp == 42) {
         /* little endian */
         get16v = _swab16;
         get32v = _swab32;
         get64v = _swab64;
	 ulSetError(UL_DEBUG, "[flt] Little endian architecture.");
      }
      else {
         /* big endian */
         get16v = _copy16;
         get32v = _copy32;
         get64v = _copy64;
	 ulSetError(UL_DEBUG, "[flt] Big endian architecture.");
      }
   }
#endif

   if (getenv("FLTNOTEX"))    NoTextures = 1;
   if (getenv("FLTNOMIPMAP")) NoMipmaps = 1;
   if (getenv("FLTNOEXT"))    NoExternals = 1;
   if (getenv("FLTNOCLEAN"))  NoClean = 1;

   return;
}

ssgEntity *ssgLoadFLT(const char *filename, 
#ifdef NO_LOADER_OPTIONS
		      ssgHookFunc
#else
		      const ssgLoaderOptions *options
#endif
		      )
{
   static int depth = 0;
   ssgEntity *node;

   if (depth == 0) {

      Init();
     
      ObsoleteFlag = 0;
      NotImplementedFlag = 0;
      
      TexCache = 0;
      StateCache = 0;
      FltCache = 0;

#ifndef NO_LOADER_OPTIONS
      ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
      LoaderOptions = ssgGetCurrentOptions () ;
#endif

   }

   // no longjmp()'s please! (or this recursion test will fail)

   depth++;

   node = LoadFLT(filename);

   depth--;

   if (depth == 0) {
     
     sfree(TexCache, S_KEY | S_DATA);
     sfree(StateCache, S_KEY);

     if (node) node->ref(); // prevent this node from being deleted!
     sfree(FltCache, S_KEY | S_TREE);
     if (node) node->deRef();

   }

#if 0
   if (node && !NoClean) {
      Flattened = 0;
      node = Flatten(node, 0);
      sfree(Flattened, 0);
   }
#endif

#if 0 /* debug */
   if (node && getenv("FLTDUMP")) {
      const char *file = "/tmp/tree.txt";
      FILE *f = fopen(file, "w");
      if (f == 0)
	 perror(file);
      else {
	 refs = 0;
	 nrefs = 0;
	 ptree(node, f, 0);
	 sfree(refs, 0);
	 fclose(f);
	 ulSetError(UL_DEBUG, "wrote %s", file);
      }
   }
#endif

   return node;
}
//lint -restore
