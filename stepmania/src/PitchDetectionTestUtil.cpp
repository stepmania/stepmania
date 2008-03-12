 /*************************************************************************/
 /*                                                                       */
 /*                Centre for Speech Technology Research                  */
 /*                     University of Edinburgh, UK                       */
 /*                      Copyright (c) 1995,1996                          */
 /*                        All Rights Reserved.                           */
 /*  Permission is hereby granted, free of charge, to use and distribute  */
 /*  this software and its documentation without restriction, including   */
 /*  without limitation the rights to use, copy, modify, merge, publish,  */
 /*  distribute, sublicense, and/or sell copies of this work, and to      */
 /*  permit persons to whom this work is furnished to do so, subject to   */
 /*  the following conditions:                                            */
 /*   1. The code must retain the above copyright notice, this list of    */
 /*      conditions and the following disclaimer.                         */
 /*   2. Any modifications must be clearly marked as such.                */
 /*   3. Original authors' names are not deleted.                         */
 /*   4. The authors' names are not used to endorse or promote products   */
 /*      derived from this software without specific prior written        */
 /*      permission.                                                      */
 /*  THE UNIVERSITY OF EDINBURGH AND THE CONTRIBUTORS TO THIS WORK        */
 /*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
 /*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
 /*  SHALL THE UNIVERSITY OF EDINBURGH NOR THE CONTRIBUTORS BE LIABLE     */
 /*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
 /*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
 /*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
 /*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
 /*  THIS SOFTWARE.                                                       */
 /*                                                                       */
 /*************************************************************************/
/*                   Author :  Paul Taylor                               */
/*                   Date   :  April 1994                                */
/*************************************************************************/

#include "global.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageSoundReader_WAV.h"
#include "RageSurface_Load.h"
#include "PitchDetectionTestUtil.h"
#include "EST_String.h"
#include "EST_Chunk.h"
#include "EST_Val.h"
#include "EST_Features.h"
#include "EST_Token.h"
#include "EST_Option.h"
#include "EST_Track.h"
#include "PitchDetectionTest.h"

// EST Library: http://www.cstr.ed.ac.uk/projects/speech_tools/
// "pda" executable options: http://festvox.org/docs/speech_tools-1.2.0/x2152.htm
// Autocorrelation explanation: http://cnx.org/content/m11714/latest/
// PDA algorithm tradeoffs: http://www-scf.usc.edu/~chinghuc/pitch_detection_algorithms.htm
// MIDI note numbers: http://tomscarff.tripod.com/midi_analyser/midi_note_frequency.htm
// MIDI note numbers: http://www.sengpielaudio.com/calculator-notenames.htm

EST_String::EST_String(const char *s) 
{
      CHECK_STRING_ARG(s);
      
      size=safe_strlen(s);

       if (size != 0)
	 memory = chunk_allocate(size+1, s, size); 
       else 
	 memory=NULL;
}

void EST_Chunk::operator delete (void *it)
{

#if defined(__CHUNK_USE_WALLOC__)
  wfree(it);
#else
  delete it;
#endif

}

val_type val_unset  = "unset";
val_type val_int    = "int";
val_type val_float  = "float";
val_type val_string = "string";

#define VAL_REGISTER_CLASS_DCLS(NAME,CLASS)            \
extern val_type val_type_##NAME;                       \
class CLASS *NAME(const EST_Val &v);                   \
EST_Val est_val(const class CLASS *v);

VAL_REGISTER_CLASS_DCLS(feats,EST_Features)


#define VAL_REGISTER_CLASS(NAME,CLASS)                 \
val_type val_type_##NAME=#NAME;                        \
class CLASS *NAME(const EST_Val &v)                    \
{                                                      \
    if (v.type() == val_type_##NAME)                   \
	return (class CLASS *)v.internal_ptr();        \
    else                                               \
	exit(1); /*EST_error("val not of type val_type_"#NAME);*/   \
    return NULL;                                       \
}                                                      \
                                                       \
static void val_delete_##NAME(void *v)                 \
{                                                      \
    delete (class CLASS *)v;                           \
}                                                      \
                                                       \
EST_Val est_val(const class CLASS *v)                  \
{                                                      \
    return EST_Val(val_type_##NAME,                    \
		   (void *)v,val_delete_##NAME);       \
}                                                      \

VAL_REGISTER_CLASS(feats,EST_Features)


EST_Val::~EST_Val(void)
{
    if ((t != val_int) &&
	(t != val_float) &&
	(t != val_unset) &&
	(t != val_string))
	delete v.pval;
}


EST_Chunk::~EST_Chunk ()
{
  if (count > 0)
    {
      //cerr << "deleting chunk with non-zero count\n";
      exit(1);
    }
  //  cerr << "deleted "<< hex << (int)&memory << "," << dec << size <<"\n";
}

void EST_Features::set_path(const EST_String &name, const EST_Val &sval)
{
    // Builds sub features (if necessary)
    
    if (strchr(name,'.') == NULL)
	set_val(name,sval);
    else
    {
	EST_String nname = name;
	EST_String fname = nname.before(".");
	if (present(fname))
	{
	    const EST_Val &v = val(fname);
	    if (v.type() == val_type_feats)
		feats(v)->set_path(nname.after("."),sval);
	    else
		exit(1);//EST_error("Feature %s not feature valued\n", 
			//  (const char *)fname);
	}
	else
	{
	    EST_Features f;
	    set(fname,f);
	    A(fname).set_path(nname.after("."),sval);
	}
    }
}



template<class K, class V> 
V &EST_TKVL<K, V>::val(const K &rkey, int must)
{ 
    EST_Litem *ptr = find_pair_key(rkey);

    if (ptr == 0)
    {
	if (must)
	  EST_error("No value set for '%s'", error_name(rkey));

	return *default_val;
    }
    else
	return list.item(ptr).v;
}

EST_String &EST_TKVL<EST_String,EST_String>::val(const EST_String &rkey, int must)
{ 
    EST_Litem *ptr = find_pair_key(rkey);

    if (ptr == 0)
    {
	if (must)
	  exit(1);//EST_error("No value set for '%s'", error_name(rkey));

	return *default_val;
    }
    else
	return list.item(ptr).v;
}


template<class K, class V> 
int EST_TKVL<K, V>::add_item(const K &rkey, const V &rval, int no_search)
{
    if (!no_search)
	if (change_val(rkey, rval)) // first see if key exists
	    return 1;
    
    EST_TKVI<K,V>  item;
    item.k = rkey;
    item.v = rval;
    
    list.append(item);
    return 1;
}

int EST_TKVL<EST_String,EST_String>::add_item(const EST_String &rkey, const EST_String &rval, int no_search)
{
    if (!no_search)
	if (change_val(rkey, rval)) // first see if key exists
	    return 1;
    
    EST_TKVI<K,V>  item;
    item.k = rkey;
    item.v = rval;
    
    list.append(item);
    return 1;
}

template<class K, class V> 
const int EST_TKVL<K, V>::present(const K &rkey) const
{ 
    if (find_pair_key(rkey) == 0)
	return 0;
    else
	return 1;
}

const int EST_TKVL<EST_String, EST_String>::present(const EST_String &rkey) const
{ 
    if (find_pair_key(rkey) == 0)
	return 0;
    else
	return 1;
}


static const EST_String Empty_String("");


int EST_Option::ival(const EST_String &rkey, int must) const
{ 
    const EST_String &tval = val_def(rkey, Empty_String);
    if (tval != "")
	return atoi(tval);

    if (must)
	exit(1);//cerr << "EST_Option: No value set for " << rkey << endl;
    return 0;
}

EST_String &EST_String::operator = (const char *str) 
{
      CHECK_STRING_ARG(str);
      int len = safe_strlen(str);
      if (!len)
	memory = NULL;
      else if (!shareing() && len < size)
	memcpy((char *)memory, str, len+1);
      else if (len)
	memory = chunk_allocate(len+1, str, len);
      size=len;
      return *this;
}

EST_String &EST_String::operator = (const char c)
{
      memory = chunk_allocate(2, &c, 1);
      size=1;
      return *this;
}

EST_String &EST_String::operator = (const EST_String &s) 
{
#if 1
  static EST_ChunkPtr hack = s.memory;
  memory = NON_CONST_CHUNKPTR(s.memory);
  size = s.size;
#else
      *(struct EST_dumb_string *)this = *(struct EST_dumb_string *)(&s);
#endif
      return *this;
}

int operator == (const char *a, const EST_String &b)
{
    CHECK_STRING_ARG(a);
      
    if (!a)
	return 0;
    else if (b.size==0)
	return *a == '\0';
    else 
	return (*a == b(0)) && strcmp(a, b.str())==0;
}

const EST_Val &EST_Features::val_path(const EST_String &name, const EST_Val &d) const
{
    // For when name contains references to sub-features
    
    if (strchr(name,'.') == NULL)
	return val(name, d);
    else
    {
	EST_String nname = name;
	EST_String fname = nname.before(".");
	const EST_Val &v = val(fname, d);
	if (v.type() == val_type_feats)
	    return feats(v)->val_path(nname.after("."), d);
	else
	    return d;
    }
}

EST_String itoString(int n)
{
    char tmp[1000];
    
    sprintf(tmp, "%d", n);
    return EST_String(tmp);
}

EST_String ftoString(float n, int pres=3, int width=0, int right_justify=0)
{
    (void)right_justify;
    EST_String val;
    char tmp[1000];
    char spec[10];
    strcpy(spec, "%");
    if (width != 0)
	strcat(spec, itoString(width));
    strcat(spec, ".");
    strcat(spec, itoString(pres));
    strcat(spec, "f");
    
    sprintf(tmp, spec, n);
    val = tmp;
    return val;
}


const EST_String &EST_Val::to_str(void) const
{
    // coerce this to and save it for later
    // This requires the following casting, so we can still tell the
    // compiler this is a const function.  If this was properly declared
    // non-const vast amounts of the rest of this would also have to be
    // non-const.  So we do one nasty bit here for uniformity elsewhere.
    // Not saving the result is also a possibility but probably too
    // inefficient (maybe not with rjc's string class)
    EST_String *n = (EST_String *)((void *)&sval);
    if (t==val_int)
	*n = itoString(v.ival);
    else if (t==val_float)
    {
	if (v.fval == 0)
	    *n = "0";  // to be compatible with other's notion of fstrings
	else
	    *n = ftoString(v.fval);
    }
    else if (t != val_string)
	*n = EST_String("[Val ")+t+"]";

    return sval;
}

int EST_Features::present(const EST_String &name) const
{
    if (strchr(name,'.') == NULL)
	return features->present(name);
    EST_String nname = name;
    if (features->present(nname.before(".")))
    {
	const EST_Val &v = val(nname.before("."));
	if (v.type() == val_type_feats)
	    return feats(v)->present(nname.after("."));
	else
	    return FALSE;
    }
    else
	return FALSE;
}

const EST_Val &EST_Features::val_path(const EST_String &name) const
{
    // For when name contains references to sub-features
    
    if (strchr(name,'.') == NULL)
	return val(name);
    else
    {
	EST_String nname = name;
	EST_String fname = nname.before(".");
	const EST_Val &v = val(fname);
	if (v.type() == val_type_feats)
	    return feats(v)->val_path(nname.after("."));
	else
	    exit(1);//EST_error("Feature %s not feature valued\n", (const char *)fname);
	return feature_default_value; // wont get here 
    }
}

EST_Track::~EST_Track(void)
{
  //  clear_features();
}

void EST_Track::default_channel_names()
{
    for (int i = 0; i < num_channels(); ++i)
	set_channel_name("track" + itoString(i), i);
}

EST_Track::EST_Track()
{
    default_vals();
}

int fastlog2(int n) {
    int num_bits, power = 0;
    
    if ((n < 2) || (n % 2 != 0)) return(0);
    num_bits = sizeof(int) * 8;	/* How big are ints on this machine? */
    
    while(power <= num_bits) {
	n >>= 1;
	power += 1;
	if (n & 0x01) {
	    if (n > 1)	return(0);
	    else return(power);
	}
    }
    return(0);
}

#define PI 3.14159265358979323846

static int slowFFTsub(EST_FVector &real, EST_FVector &imag, float f) 
{
    // f = -1 for FFT, 1 for IFFT
    // would be nicer if we used a complex number class, 
    // but we don't, so it isn't

    // taken from the FORTRAN old chestnut
    // in various sig proc books
    // FORTRAN uses 1..n arrays, so subtract 1 all over the place


    float u_real,u_imag;
    float w_real,w_imag;
    float t_real,t_imag;
    float tmp_real,tmp_imag;

    int M,N;
    int i,j,k,l;
    
    M = fastlog2(real.n());
    N = (int)powf(2,(float)M);

    if (N != real.n())
      {
	exit(1);//EST_warning("Illegal FFT order %d", real.n());
	return -1;
      }

    for(l=1;l<=M;l++){

	int le = (int)powf(2,(float)(M+1-l));
	int le1=le/2;

	u_real = 1.0;
	u_imag = 0.0;

	w_real=cos(PI/le1);
	w_imag=f * sin(PI/le1);

	for (j=1;j<=le1;j++)
	{
	    for (i=j;i<=N-le1;i+=le)
	    {
		int ip=i+le1;
		t_real = real.a_no_check(i-1) + real.a_no_check(ip-1);
		t_imag = imag.a_no_check(i-1) + imag.a_no_check(ip-1);

		tmp_real = real.a_no_check(i-1) - real.a_no_check(ip-1);
		tmp_imag = imag.a_no_check(i-1) - imag.a_no_check(ip-1);

		real.a_no_check(ip-1) = tmp_real*u_real - tmp_imag*u_imag;
		imag.a_no_check(ip-1) = tmp_real*u_imag + tmp_imag*u_real;

		real.a_no_check(i-1) = t_real;
		imag.a_no_check(i-1) = t_imag;
	    }

	    tmp_real = u_real*w_real - u_imag*w_imag;
	    tmp_imag = u_real*w_imag + u_imag*w_real;
	    
	    u_real=tmp_real;
	    u_imag=tmp_imag;

	}

    }


    int NV2=N/2;
    int NM1=N-1;
    j=1;


    for (i=1; i<=NM1;i++)
    {
	if (i < j)
	{
	    t_real=real(j-1);
	    t_imag=imag(j-1);
	    
	    real[j-1] = real(i-1);
	    imag[j-1] = imag(i-1);

	    real[i-1] = t_real;
	    imag[i-1] = t_imag;
	    
	}

	k=NV2;

	while(k < j)
	{
	    j=j-k;
	    k=k/2;
	}

	j=j+k;

    }

    return 0;
}

int slowIFFT(EST_FVector &real, EST_FVector &imag) 
{
    int N=real.n();
    if (N <=0 )
	return -1;

    if (slowFFTsub(real,imag,1.0) != 0)
	return -1;

    for(int i=1;i<=N;i++){
	real[i-1] /= (float)N;
	imag[i-1] /= (float)N;
    }

    return 0;
}


EST_FVector design_FIR_filter(const EST_FVector &frequency_response, 
			      int filter_order)
{
    // frequency_response contains the desired filter reponse,
    // on a scale 0...sampling frequency
    
    // check filter_order is odd
    if((filter_order & 1) == 0){
	cerr << "Requested filter order must be odd" << endl;
	return EST_FVector(0);
    }
	
    // check frequency_response has dimension 2^N
    int N = fastlog2(frequency_response.n());
    if(frequency_response.n() !=  (int)powf(2,(float)N)){
	cerr << "Desired frequency response must have dimension 2^N" << endl;
	return EST_FVector(0);
    }

    int i;
    EST_FVector filt(frequency_response);
    EST_FVector dummy(frequency_response.n());
    for(i=0;i<dummy.n();i++)
	dummy[i] = 0.0;

    int e=slowIFFT(filt,dummy);
    if (e != 0)
    {
	cerr << "Failed to design filter because FFT failed" << endl;
	return EST_FVector(0);
    }

    EST_FVector reduced_filt(filter_order);

    int mid = filter_order/2;

    reduced_filt[mid] = filt(0);
    for(i=1; i<=mid ;i++)
    {
	// Hann window for zero ripple
	float window =  0.5 + 0.5 * cos(PI*(float)i / (float)mid);
	reduced_filt[mid+i] = filt(i) * window;
	reduced_filt[mid-i] = filt(i) * window;
    }

    return reduced_filt;
}

EST_FVector design_high_or_low_pass_FIR_filter(int sample_rate, 
					       int cutoff_freq, int order,
					       float gain1, float gain2)
{
    // change to bandpass filter .....
    
    if (sample_rate <= 0){
	cerr << "Can't design a FIR filter for a sampling rate of "
	    << sample_rate << endl;
	return EST_FVector(0);
    }
    
    int i;
    int N=10;			// good minimum size
    
    int fft_size = (int)pow(2.0, N);
    while(fft_size < order*4){	// rule of thumb !?
	N++;
	fft_size = (int)pow(2.0, N);
    }
    
    // freq response is from 0 to sampling freq and therefore
    // must be symmetrical about 1/2 sampling freq
    
    EST_FVector freq_resp(fft_size);
    int normalised_cutoff = (fft_size * cutoff_freq)/sample_rate;
    for(i=0;i<normalised_cutoff;i++){
	freq_resp[i] = gain1;
	freq_resp[fft_size-i-1] = gain1;
    }
    for(;i<fft_size/2;i++){
	freq_resp[i] = gain2;
	freq_resp[fft_size-i-1] = gain2;
    }
    
    return design_FIR_filter(freq_resp, order);
}


EST_FVector design_lowpass_FIR_filter(int sample_rate, int freq, int order)
{
    return design_high_or_low_pass_FIR_filter(sample_rate, 
					      freq, order, 1.0, 0.0);
}


struct  Ms_Op {			/* median smoother operations */
  int smooth_double;
  int apply_hanning;
  int extrapolate;
  int first_median;
  int second_median;
  int window_length;
  int interp;
  float breaker;
};

struct Ms_Op *default_ms_op(struct Ms_Op *ms)
{
    ms->smooth_double = FALSE;
    ms->apply_hanning = TRUE;
    ms->extrapolate = TRUE;
    ms->first_median = 11;
    ms->second_median = 1; 
    ms->window_length = 7; 
    ms->breaker = -1.0;
    return (ms);
}

int parse_ms_list(EST_Features &al, struct Ms_Op *ms)
{
    default_ms_op(ms);
    
    if (al.present("smooth_double"))
	ms->smooth_double = al.I("smooth_double");
    if (al.present( "hanning"))
	ms->apply_hanning = al.I("hanning");
    if (al.present("extrapolate"))
	ms->extrapolate = al.I("extrapolate");
    if (al.present("first_length"))
	ms->first_median = al.I("first_length");
    if (al.present("second_length"))
	ms->second_median = al.I("second_length");
    if (al.present("window_length"))
	ms->window_length = al.I("window_length");

    return 0;
}

#define MAX_LEN             127

#define TWO_PI 6.28318530717958647698

void mk_window_coeffs (int length, float win_coeff[])
{
    int i;
    double x;

    for (i = 0; i < length; i++) {
	x = TWO_PI * (i + 1.0) / (length + 1.0);
	win_coeff[i] = (1.0 - (float) cos (x)) / (length + 1.0);
    }

}

float median (int *counter, float valin, float valbuf[], int lmed, int mmed)
{
    int i, j;
    float tmp, filmed[MAX_LEN];

    for (i = lmed - 1; i > 0; i--)
	valbuf[i] = valbuf[i - 1];
    valbuf[0] = valin;

    if (*counter > 0) 
    {
	(*counter)--;
	return (0.0);
    }
    else 
    {
	*counter = -1;

	for (i = 0; i < lmed; i++)
	    filmed[i] = valbuf[i];

	for (j = lmed - 1; j > 0; j--)
	    for (i = 0; i < j; i++)
		if (filmed[i] > filmed[i + 1]) 
		{
		    tmp = filmed[i + 1];
		    filmed[i + 1] = filmed[i];
		    filmed[i] = tmp;
		}
	return (filmed[mmed]);
    }

}

float hanning (int *counter, float valin, float valhan[], float win_coeff[], 
	       struct Ms_Op *par)
{
    int i, j, k = 0;
    float valout = 0.0, weight[MAX_LEN];

    for (i = par->window_length - 1; i > 0; i--)
	valhan[i] = valhan[i - 1];
    valhan[0] = valin;
    if (*counter > 0) {
	(*counter)--;
	return (0.0);
    }
    else {
	*counter = -1;
	for (i = 0; i < par->window_length; i++)
	    if (valhan[i] == par->breaker)
		k++;
	if (!k)
	    for (i = 0; i < par->window_length; i++)
		valout += valhan[i] * win_coeff[i];
	else if (k <= par->window_length / 2 && par->extrapolate) {
	    mk_window_coeffs (par->window_length - k, weight);
	    for (i = 0, j = 0; i < par->window_length; i++)
		if (valhan[i] != par->breaker)
		    valout += valhan[i] * weight[j++];
	}
	else
	    valout = par->breaker;
	return (valout);
    }

}


void array_smoother (float *p_array, int arraylen, struct Ms_Op *ms)
{
    int i, j, mid1, mid2 = 0, filler, nloops;
    int C1, C2 = 0, C3 = 0, C4 = 0, c1, c2, c3, c4;
    int delay, delx = 0, dely = 0;
    int in = 0, out = 0;
    float input, output;
    float *inarray;
    float xdel[2 * MAX_LEN - 2], ydel[2 * MAX_LEN - 2];
    float medbuf1[MAX_LEN], medbuf2[MAX_LEN];
    float hanbuf1[MAX_LEN], hanbuf2[MAX_LEN], win_coeffs[MAX_LEN];
    float medval1, medval2, hanval1, hanval2, zatn;

    inarray = new float[arraylen];
    for (i = 0; i < arraylen; ++i)
	inarray[i] = p_array[i];

    if (ms == NULL)
    { 
	ms = new Ms_Op;
	default_ms_op(ms);
    }

    mk_window_coeffs (ms->window_length, win_coeffs);
    /* determine the size and delay of each stage concerned */
    mid1 = ms->first_median / 2;
    C1 = delay = ms->first_median - 1;
    if (ms->apply_hanning) 	
    {
	C2 = ms->window_length - 1;
	delay = ms->first_median + ms->window_length - 2;
    }
    if (ms->smooth_double) {
	mid2 = ms->second_median / 2;
	C3 = ms->second_median - 1;
	if (!ms->apply_hanning) {
	    delx = ms->first_median;
	    dely = ms->second_median;
	}
	else {
	    C4 = ms->window_length - 1;
	    delx = ms->first_median + ms->window_length - 1;
	    dely = ms->second_median + ms->window_length - 1;
	}
	delay = delx + dely - 2;
    }
    /* prepare for smoothing */
    c1 = C1;
    c2 = C2;
    c3 = C3;
    c4 = C4;
    if (!ms->extrapolate) {
	/* pad with breakers at the beginning */
	for (i = 0; i < delay / 2; i++)
	    p_array[out++] = ms->breaker; 
	filler = 0;
	nloops = arraylen;
    }
    else {
	/* extrapolate by initialising filter with dummy breakers */
	filler = delay / 2;
	nloops = arraylen + delay;
    }
    /* smooth track element by track element */
    for (j = 0; j < nloops; j++) 
    {
	if (j < filler || j >= nloops - filler)
	    input = ms->breaker;
	else
	    input = inarray[in++];

	/* store input value if double smoothing */
	if (ms->smooth_double) {
	    for (i = delx - 1; i > 0; i--)
		xdel[i] = xdel[i - 1];
	    xdel[0] = input;
	}
	/* first median smoothing */

	medval1 = median (&c1, input, medbuf1, ms->first_median, mid1);

	if (c1 == -1) 
	{
	    output = medval1;
	    /* first hanning window (optional) */
	    if (ms->apply_hanning) 
	    {
		hanval1 = hanning (&c2, medval1, hanbuf1, win_coeffs, ms);
		if (c2 == -1)
		    output = hanval1;
		else
		    continue;
	    }
	    /* procedures for double smoothing (optional) */
	    if (ms->smooth_double) 
	    {
		/* compute rough component z(n) */
		if (output != ms->breaker && xdel[delx - 1] 
		    != ms->breaker)
		    zatn = xdel[delx - 1] - output;
		else
		    zatn = ms->breaker;
		/* store results of first smoothing */
		for (i = dely - 1; i > 0; i--)
		    ydel[i] = ydel[i - 1];
		ydel[0] = output;
		/* second median smoothing */
		medval2 = median (&c3, zatn, medbuf2, 
				  ms->second_median, mid2);
		if (c3 == -1) 
		{
		    output = medval2;
		    /* second hanning smoothing (optional) */
		    if (ms->apply_hanning) {
			hanval2 = hanning (&c4, medval2, hanbuf2, 
					   win_coeffs, ms);
			if (c4 == -1)
			    output = hanval2;
			else
			    continue;
		    }
		    if (output != ms->breaker && ydel[dely - 1] 
			!= ms->breaker)
			output += ydel[dely - 1];
		    else
			output = ms->breaker;
		}
		else
		    continue;
	    }
	    /* write filtered result */
	    p_array[out++] = output;
	}
    }
    if (!ms->extrapolate) 	/* pad with breakers at the end */
	for (i = 0; i < delay / 2; i++)
	    p_array[out++] = ms->breaker;

    delete inarray;
}

void smooth_portion(EST_Track &c, EST_Features &op)
{
    int i;
    float *a;  // need float * so it can be passed to array_smoother
    struct Ms_Op *ms;
    ms = new Ms_Op;

    default_ms_op(ms);
    parse_ms_list(op, ms);

    if (op.present("point_window_size"))
	ms->window_length = op.I("point_window_size");

    a = new float[c.num_frames()];
    
    for (i = 0; i < c.num_frames(); ++i)
	a[i] = c.track_break(i) ? -1.0 : c.a(i);

    array_smoother(a, c.num_frames(), ms);

    for (i = 0; i < c.num_frames(); ++i)
    {   // occasionally NaNs result...
	if (isnanf(a[i]))
	{
	    c.set_break(i);
	    c.a(i) = 0.0;
	}
	else
	{
	    if (a[i] < 0.0)
		c.set_break(i);
	    else
		c.set_value(i);
	    c.a(i) = a[i];
	}
    }

    delete a;
}

static void interp(const EST_Track &c, const EST_Track &speech, int fill,
		   EST_Track &interp)
{
    // Interpolate between unvoiced sections, and ensure breaks
    // during silences
    int i, n, p;
    float m;
    float n_val, p_val;
    float f = c.shift();

    interp = c;  // copy track

    if (speech.num_frames() < c.num_frames())
	interp.resize(speech.num_frames(), interp.num_channels());


    for (i = 1; i < interp.num_frames(); ++i)
    {
	if ((fill == 1) || (speech.a(i) > 0.5))
	{
	    if (!interp.track_break(i))
		continue;  // already has a value

	    p = i - 1;
	    if ((n = interp.next_non_break(i)) == 0)
		n = interp.num_frames() - 1;
	    n_val = interp.a(n);
	    p_val = interp.a(p);
	    if (n_val <= 0) n_val = p_val;
	    if (p_val <= 0) p_val = n_val;
	    // if they are both zero, well we'll learn to live it.
	    m = (n_val - p_val) / ( interp.t(n) - interp.t(p));

	    interp.a(i) = (m * f) + p_val;
	    interp.set_value(i);
	}
	else
	    interp.set_break(i);
    }
}

void smooth_phrase(EST_Track &fz, EST_Track &speech, EST_Features &op, 
		   EST_Track &smi_fz)
{
    int n=0;
    EST_Track sm_fz;
    char nstring[10];

    if (fz.empty())
    {
	smi_fz = fz;
	return;
    }
    sm_fz = fz;
    sm_fz.set_channel_name("F0", 0);

    n = (int)(op.F("window_length") / fz.shift());
    sprintf(nstring, "%d", n);
    op.set("point_window_size", nstring);

    if (!op.present("icda_no_smooth"))
	smooth_portion(sm_fz, op);

    if (op.present("icda_no_interp"))
    {
	sm_fz = fz;
	return; // no unvoiced interpolation
    }

    int fill = op.present("icda_fi") ? 1 : 0;
    interp(sm_fz, speech, fill, smi_fz); // fill unvoiced region

    n = (int)(op.F("second_length") / fz.shift());
    sprintf(nstring, "%d", n);
    op.set("point_window_size", nstring);

    if (!op.present("icda_no_smooth"))
	smooth_portion(smi_fz, op);
}

const int EST_Val::to_int(void) const
{
    // coerce this to an int
    if (t==val_float)
	return (int)v.fval;
    else if (t==val_string)
	return atoi(sval);
    else
	return v.ival;  // just for completeness
}

struct SEGMENT_
{                    /* segment of speech data */
  int size, shift, length;          /* in samples */
  short *data;
};

struct CROSS_CORR_
{
  int size;
  double *coeff;
};


void end_structure_use(SEGMENT_ *p_seg, CROSS_CORR_ *p_cc)
{

  wfree (p_seg->data);
  wfree (p_cc->coeff);
  return;

}

static bool bounds_check(const EST_Track &t, int f, int c, int set)
{
  const char *what = set? "set" : "access";

  if (f<0 || f >= t.num_frames())
    {
      cerr << "Attempt to " << what << " frame " << f << " of " << t.num_frames() << " frame track\n";
      return FALSE;
    }
  if (c<0 || c >= t.num_channels())
    {
      cerr << "Attempt to " << what << " channel " << c << " of " << t.num_channels() << " channel track\n";
      return FALSE;
    }

return TRUE;
}


float &EST_Track::a(int i, int c)
{
  if (!bounds_check(*this, i,c,0))
      return *(p_values.error_return);

  return p_values.a_no_check(i,c);
}

EST_ChunkPtr chunk_allocate(int bytes, const char *initial, int initial_len)
{
  if (initial_len >= bytes)
    {
      cerr<<"initialiser too long\n";
      abort();
    }

  EST_Chunk *cp = new(bytes) EST_Chunk;

  memcpy(cp->memory, initial, initial_len);
  
  cp->memory[initial_len] = '\0';

  return (EST_ChunkPtr)cp;
}

void wfree(void *p)
{
    if (p != NULL)
	free(p);
}

EST_Features::EST_Features()
{
    features = new EST_TKVL<EST_String, EST_Val>;
}

EST_Features::~EST_Features()
{
  if (features != NULL)
    {
      delete features;
      features=NULL;
    }
}

const EST_Val &EST_Features::val(const char *name) const
{
    // Because so many access are from char* literals we all access
    // directly rather than requiring the creation of an EST_String
    EST_Litem *p;

    for (p=features->list.head(); p; p=next(p))
    {
	if (features->list(p).k == name)
	    return features->list(p).v;
    }

    exit(1);//EST_error("{FND} Feature %s not defined\n", name);
    return feature_default_value;
}

EST_String EST_String::chop_internal(const char *it, int len, int from, EST_chop_direction mode) const
{
  CHECK_STRING_ARG(it);
      
  int start, end;
  
  if (it && locate(it, len, from, start, end))
    switch (mode)
      {
      case Chop_Before:
	return EST_String(str(), size, 0, start); break;
      case Chop_At:
	return EST_String(str(), size, start, end-start); break;
      case Chop_After:
	return EST_String(str(), size, end, -1);
      }
  return EST_String();

}

EST_Val::EST_Val(val_type type,void *p, void (*f)(void *))
{
    t=type;
    v.pval = new EST_Contents;
    v.pval->set_contents(p,f);
}

EST_Features::EST_Features(const EST_Features &f)
{
    features = new EST_TKVL<EST_String, EST_Val>;
    *features = *f.features;
}

void EST_TokenStream::close(void)
{
    // close any files (if they were used)
    
    switch (type)
    {
      case tst_none: 
	break;
      case tst_file:
	if (close_at_end)
	  fclose(fp);
      case tst_pipe:
	// close(fd);
	break;
      case tst_istream:
	break;
      case tst_string:
	delete [] buffer;
	buffer = 0;
	break;
      default:
	cerr << "EST_TokenStream: unknown type" << endl;
	break;
    }

    type = tst_none;
    peeked_charp = FALSE;
    peeked_tokp = FALSE;

}

EST_TokenStream::~EST_TokenStream()
{
    if (type != tst_none) 
	close();
    delete [] tok_wspace;
    delete [] tok_stuff;
    delete [] tok_prepuncs;
    
}

int EST_TokenStream::open(const EST_String &filename)
{
    if (type != tst_none)
	close();
    default_values();
    fp = fopen(filename,"rb");
    if (fp == NULL)
    {
	exit(1);//cerr << "Cannot open file " << filename << " as tokenstream" 
	//    << endl;
	return -1;
    }
    Origin = filename;
    type = tst_file;

    return 0;
}

const EST_String Token_Origin_FD = "existing file descriptor";

int EST_TokenStream::open(FILE *ofp, int close_when_finished)
{
    // absorb already open stream
    if (type != tst_none)
	close();
    default_values();
    fp = ofp;
    if (fp == NULL)
    {
	cerr << "Cannot absorb NULL filestream as tokenstream" << endl;
	return -1;
    }
    Origin = Token_Origin_FD;
    type = tst_file;
    
    close_at_end = close_when_finished;
    
    return 0;
}

EST_TokenStream::EST_TokenStream()
{
    tok_wspacelen = 64;  // will grow if necessary
    tok_wspace = new char[tok_wspacelen];
    tok_stufflen = 512;  // will grow if necessary
    tok_stuff = new char[tok_stufflen];
    tok_prepuncslen = 32;  // will grow if necessary
    tok_prepuncs = new char[tok_prepuncslen];

    default_values();
}











#define Instantiate_KVL_T(KEY, VAL, TAG) \
        template class EST_TKVL<KEY, VAL>; \
        template class EST_TKVI<KEY, VAL>; \
	ostream &operator<<(ostream &s, EST_TKVI< KEY , VAL > const &i){  return s << i.k << "\t" << i.v << "\n"; } \
	ostream& operator << (ostream& s,  EST_TKVL< KEY , VAL > const &l) {EST_Litem *p; for (p = l.list.head(); p ; p = next(p)) s << l.list(p).k << "\t" << l.list(p).v << endl; return s;} \
        Instantiate_TIterator_T(KVL_ ## TAG ## _t, KVL_ ## TAG ## _t::IPointer_k, KEY, KVL_ ## TAG ##_kitt) \
        Instantiate_TStructIterator_T(KVL_ ## TAG ## _t, KVL_ ## TAG ## _t::IPointer, KVI_ ## TAG ## _t, KVL_ ## TAG ##_itt) \
        Instantiate_TIterator_T(KVL_ ## TAG ## _t, KVL_ ## TAG ## _t::IPointer, KVI_ ## TAG ## _t, KVL_ ## TAG ##_itt) \
        Instantiate_TList(KVI_ ## TAG ## _t)

// template ostream & operator<<(ostream &s, EST_TKVI<KEY, VAL> const &i); 

#define Instantiate_KVL(KEY, VAL) \
		Instantiate_KVL_T(KEY, VAL, KEY ## VAL) 

#define Declare_KVL_TN(KEY, VAL, MaxFree, TAG) \
	typedef EST_TKVI<KEY, VAL> KVI_ ## TAG ## _t; \
	typedef EST_TKVL<KEY, VAL> KVL_ ## TAG ## _t; \
	\
	static VAL TAG##_kv_def_val_s; \
	static KEY TAG##_kv_def_key_s; \
	\
	template <> VAL *EST_TKVL< KEY, VAL >::default_val=&TAG##_kv_def_val_s; \
	template <> KEY *EST_TKVL< KEY, VAL >::default_key=&TAG##_kv_def_key_s; \
	\
	Declare_TList_N(KVI_ ## TAG ## _t, MaxFree)
#define Declare_KVL_T(KEY, VAL, TAG) \
	Declare_KVL_TN(KEY, VAL, 0, TAG)

#define Declare_KVL_Base_TN(KEY, VAL, DEFV, DEFK, MaxFree, TAG) \
	typedef EST_TKVI<KEY, VAL> KVI_ ## TAG ## _t; \
	typedef EST_TKVL<KEY, VAL> KVL_ ## TAG ## _t; \
	\
	static VAL TAG##_kv_def_val_s=DEFV; \
	static KEY TAG##_kv_def_key_s=DEFK; \
	\
	template <> VAL *EST_TKVL< KEY, VAL >::default_val=&TAG##_kv_def_val_s; \
	template <> KEY *EST_TKVL< KEY, VAL >::default_key=&TAG##_kv_def_key_s; \
	\
	Declare_TList_N(KVI_ ## TAG ## _t, MaxFree)
#define Declare_KVL_Base_T(KEY, VAL, DEFV, DEFK, TAG) \
	Declare_KVL_Base_TN(KEY, VAL, DEFV, DEFK, 0, TAG)

#define Declare_KVL_Class_TN(KEY, VAL, DEFV, DEFK, MaxFree, TAG) \
	typedef EST_TKVI<KEY, VAL> KVI_ ## TAG ## _t; \
	typedef EST_TKVL<KEY, VAL> KVL_ ## TAG ## _t; \
	\
	static VAL TAG##_kv_def_val_s(DEFV); \
	static KEY TAG##_kv_def_key_s(DEFK); \
	\
	template <> VAL *EST_TKVL< KEY, VAL >::default_val=&TAG##_kv_def_val_s; \
	template <> KEY *EST_TKVL< KEY, VAL >::default_key=&TAG##_kv_def_key_s; \
	\
	Declare_TList_N(KVI_ ## TAG ## _t, MaxFree)
#define Declare_KVL_Class_T(KEY, VAL, DEFV, DEFK,TAG) \
	Declare_KVL_Class_TN(KEY, VAL, DEFV, DEFK, 0, TAG)

#define Declare_KVL_N(KEY, VAL, MaxFree) \
		Declare_KVL_TN(KEY, VAL, MaxFree, KEY ## VAL)
#define Declare_KVL(KEY, VAL) \
		Declare_KVL_N(KEY, VAL, 0)

#define Declare_KVL_Base_N(KEY, VAL, DEFV, DEFK, MaxFree)  \
		Declare_KVL_Base_TN(KEY, VAL, DEFV, DEFK, , MaxFree, KEY ## VAL)
#define Declare_KVL_Base(KEY, VAL, DEFV, DEFK)  \
		Declare_KVL_Base_N(KEY, VAL, DEFV, DEFK, 0)

#define Declare_KVL_Class_N(KEY, VAL, DEFV, DEFK, MaxFree) \
		Declare_KVL_Class_TN(KEY, VAL, DEFV, DEFK, MaxFree, KEY ## VAL)
#define Declare_KVL_Class(KEY, VAL, DEFV, DEFK) \
		Declare_KVL_Class_N(KEY, VAL, DEFV, DEFK, 0)



Declare_KVL(EST_String,EST_String)

template <class K, class V> 
EST_Litem *EST_TKVL<K, V>::find_pair_key(const K &key) const
{
    EST_Litem *ptr;

    for (ptr = list.head(); ptr != 0; ptr= next(ptr))
	if (list.item(ptr).k == key)
	    return ptr;
    return 0;
}

// look for key rkey in list. If found, change its value to rval and
// return true, otherwise return false.
template <class K, class V> 
int EST_TKVL<K, V>::change_val(const K &rkey,const V &rval)
{
    EST_Litem *ptr=find_pair_key(rkey);
    if (ptr == 0)
	return 0;
    else
    {
	list.item(ptr).v = rval;
	return 1;
    }
}

template<class K, class V> 
const V &EST_TKVL<K, V>::val_def(const K &rkey, const V &def) const
{ 
    EST_Litem *ptr = find_pair_key(rkey);
    if (ptr == 0)
	return def;
    else
	return list.item(ptr).v;
}

void make_updatable(EST_ChunkPtr &cp)
{
  if (cp.ptr && cp.ptr->count > 1)
    {
      EST_Chunk *newchunk = new(cp.ptr->size) EST_Chunk;

      memcpy(newchunk->memory, cp.ptr->memory, cp.ptr->size);

      cp = newchunk;
    }
}

const EST_Val &EST_Features::val(const char *name, const EST_Val &def) const
{
    // Because so many access are from char* literals we all access
    // directly rather than requiring the creation of an EST_String
    EST_Litem *p;

    for (p=features->list.head(); p; p=next(p))
    {
	if (features->list(p).k == name)
	    return features->list(p).v;
    }
    return def;
}

EST_String operator + (const EST_String &a, const char *b)
{
  CHECK_STRING_ARG(b);

    int al = a.size;
    int bl = safe_strlen(b);

    if (al == 0)
      return EST_String(b, 0, bl);
    if (bl == 0)
      return EST_String(a);

    EST_ChunkPtr c = chunk_allocate(al+bl+1, a.str(), al);

    if (bl>0)
      memmove((char *)c + al, b, bl);
    c(al+bl)='\0';

    return EST_String(al+bl, c);
}

EST_Val EST_Features::feature_default_value("0");

EST_Featured::~EST_Featured(void)
{
  clear_features();
}

template<class T>
EST_TMatrix<T>::~EST_TMatrix()
{
    p_num_rows = 0;
    p_row_step=0;
}

template<class T>
EST_TVector<T>::~EST_TVector()
{
  p_num_columns = 0;
  p_offset=0;
  p_column_step=0;

  if (p_memory != NULL && !p_sub_matrix)
    {
      delete [] (p_memory-p_offset);
      p_memory = NULL;
    }
}

void EST_Track::set_channel_name(const EST_String &fn, int i)
{
    p_channel_names[i] = fn;
}    

EST_String operator + (const char *a, const EST_String &b)
{
  CHECK_STRING_ARG(a);

    int al = safe_strlen(a);
    int bl = b.size;

    if (bl == 0)
      return EST_String(a, 0, al);
    if (al == 0)
      return EST_String(b);

    EST_ChunkPtr c = chunk_allocate(al+bl+1, a, al);

    memmove((char *)c + al, b.str(), bl);

    c(al+bl)='\0';

    return EST_String(al+bl, c);
}

void EST_Track::default_vals(void)
{
    p_equal_space = FALSE;
    p_single_break = FALSE;
    p_values.resize(0, 0);
    p_times.resize(0);
    p_is_val.resize(0);
    p_aux.resize(0, 0);
    p_aux_names.resize(0);
    p_channel_names.resize(0);
    p_map = NULL;
    p_t_offset=0;
    
    init_features();
}

template<class T>
EST_TVector<T>::EST_TVector()
{
    default_vals();
}

template<class T>
EST_TMatrix<T>::EST_TMatrix()
{
  default_vals();
}

EST_Featured::EST_Featured(void)
{
  init_features();
}

template<class T> EST_TSimpleVector<T>::EST_TSimpleVector(const EST_TSimpleVector<T> &in)
{
    this->default_vals();
    copy(in);
}

void *safe_walloc(int size)
{
    char *p;
    
    if (size == 0)
	/* Some mallocs return NULL for size 0, which means you can't tell
	   if it failed or not. So we'll avoid that problem by never 
	   asking for 0 bytes */
	p = (char*)calloc(1,1);
    else
	p = (char*)calloc(size,1);

    if (p == NULL)
    {
	fprintf(stderr,"WALLOC: failed to malloc %d bytes\n",size);
	exit(-1);  /* I'd rather not do this but this is the only safe */
	           /* thing to do */
    }

    return p;
}

/* return the lesser of the two values  */
#define Lof(a, b) (((a) < (b)) ? (a) : (b))

template<class T> 
void EST_TSimpleMatrix<T>::resize(int new_rows, 
				  int new_cols, 
				  int set)
{
  T* old_vals=NULL;
  int old_offset = this->p_offset;

  if (new_rows<0)
    new_rows = this->num_rows();
  if (new_cols<0)
    new_cols = this->num_columns();

  if (set)
    {
      if (!this->p_sub_matrix && new_cols == this->num_columns() && new_rows != this->num_rows())
	{
	  int copy_r = Lof(this->num_rows(), new_rows);

	  just_resize(new_rows, new_cols, &old_vals);

	  memcpy((void *)this->p_memory, 
		 (const void *)old_vals,
		 copy_r*new_cols*sizeof(T));
	  
	  int i,j;
	  
	  if (new_rows > copy_r)
	    if (*this->def_val == 0)
	      {
		memset((void *)(this->p_memory + copy_r*this->p_row_step),
		       0,
		       (new_rows-copy_r)*new_cols*sizeof(T));
	      }
	    else
	      {
		for(j=0; j<new_cols; j++)
		  for(i=copy_r; i<new_rows; i++)
		    this->a_no_check(i,j) = *this->def_val;
	      }
	}
      else if (!this->p_sub_matrix)
	{
	  int old_row_step = this->p_row_step;
	  int old_column_step = this->p_column_step;
	  int copy_r = Lof(this->num_rows(), new_rows);
	  int copy_c = Lof(this->num_columns(), new_cols);
	  
	  just_resize(new_rows, new_cols, &old_vals);

	  set_values(old_vals,
		     old_row_step, old_column_step,
		     0, copy_r,
		     0, copy_c);

	  int i,j;
	  
	  for(i=0; i<copy_r; i++)
	    for(j=copy_c; j<new_cols; j++)
	      this->a_no_check(i,j) =  *this->def_val;
	  
	  if (new_rows > copy_r)
	    if (*this->def_val == 0)
	      {
		memset((void *)(this->p_memory + copy_r*this->p_row_step),
		       0,
		       (new_rows-copy_r)*new_cols*sizeof(T));
	      }
	    else
	      {
		for(j=0; j<new_cols; j++)
		  for(i=copy_r; i<new_rows; i++)
		    this->a_no_check(i,j) = *this->def_val;
	      }
	}
      else
	EST_TMatrix<T>::resize(new_rows, new_cols, 1);
    }
  else
    EST_TMatrix<T>::resize(new_rows, new_cols, 0);

  if (old_vals && old_vals != this->p_memory)
    delete [] (old_vals-old_offset);
}


void EST_Track::set_value(int i) // make location i hold a value
{
    p_is_val[i] = 0;
}    

void EST_Track::set_break(int i) // make location i hold a break
{
    if (i >= num_frames())
	cerr << "Requested setting of break value of the end of the array\n";
    
    p_is_val[i] = 1;
}    

float EST_Track::shift() const
{
    int j1 = 0;
    int j2 = 0;
    
    if (!p_equal_space)
	exit(1);//EST_error("Tried to take shift from non-fixed contour\n");

    do
    {
	j1 = next_non_break(++j1);
	j2 = next_non_break(j1);
	//	cout << "j1:" << j1 << " j2:" << j2 << endl;
    }
    while ((j2 != 0) && (j2 != (j1 +1)));
    
    if (j2 == 0)
    {
	if (num_frames() > 1)
	    return p_times(1) - p_times(0);
	else
	    exit(1);//EST_error("Couldn't determine shift size\n");	    

    }
    return (p_times(j2) - p_times(j1));
}

EST_Track &EST_Track::operator=(const EST_Track& a)
{
    copy(a);
    return *this;
}    

int EST_Track::empty() const
{
    int i, num;
    
    for (i = num = 0; i < num_frames(); ++i)
	if (val(i))
	    return 0;		// i.e. false
    
    return 1;			// i.e. true
}

const float EST_Val::to_flt(void) const
{
    // coerce this to a float
    if (t==val_int)
	return (float)v.ival;
    else if (t==val_string)
	return atof(sval);
    else
	return v.fval;  // just for completeness
}

int EST_Track::next_non_break(int j) const
{
    int i = j;
    for (++i; i < num_frames(); ++i)
    {
	//	cout << "i: " << i << " " << value[i] << endl;
	if (!track_break(i))
	    return i;
    }
    
    return 0;
}

float EST_Track::a(int i, int c) const
{
  return ((EST_Track *)this)->a(i,c);
}

void EST_Track::resize(int new_num_frames, int new_num_channels, bool set)
{
    int old_num_frames = num_frames();

    if (new_num_frames<0)
	new_num_frames = num_frames();

    if (new_num_channels<0)
	new_num_channels = num_channels();

    p_channel_names.resize(new_num_channels);

    // this ensures the new channels have a default name
    if (new_num_channels > num_channels())
	for (int i = num_channels(); i < new_num_channels; ++i)
	    set_channel_name("track_" + itoString(i), i);

    p_values.resize(new_num_frames, new_num_channels, set);
    p_times.resize(new_num_frames, set);
    p_is_val.resize(new_num_frames, set);

    p_aux.resize(new_num_frames, num_aux_channels(), set);
  
    // Its important that any new vals get set to 0
    for (int i = old_num_frames; i < num_frames(); ++i)
	p_is_val.a_no_check(i) = 0;

}

Declare_TVector(float)

EST_Chunk::EST_Chunk ()
{
  count = 0;
  memory[0] = '\0';
  //  cerr<<"created " << hex << (int)&memory << "," << dec << size <<"\n";
}

void *EST_Chunk::operator new (size_t size, int bytes)
{

  if (bytes > MAX_CHUNK_SIZE)
    {
      cerr<<"trying to make chunk of size "<<bytes<<"\n";
    }

#if defined(__CHUNK_USE_WALLOC__)
  void *it = walloc(char, size+bytes);
#else
  void *it = new char[size + bytes];
#endif

  //  cerr<<"allocated "<<bytes+size<<" byte for chunk\n";

  ((EST_Chunk *)it) -> size = bytes;

  return it;
}

EST_String::EST_String(const char *s, int s_size, int start, int len) 
{
  CHECK_STRING_ARG(s);
      
  if (len <0)
    len=s_size-start;

  size=len;
  if (size != 0)
    memory = chunk_allocate(len+1, s+start, len);
  else
    memory=NULL;
}

int EST_String::locate(const char *s, int len, int from, int &start, int &end) const
{
  CHECK_STRING_ARG(s);
      
  const char *sub=NULL;

  if (!s)
    return 0;

  if (from < 0 && -from < size)
    {
      int endpos=size+from+1;
      int p=0;
      const char *nextsub;

      while ((nextsub=strstr(str()+p, s)))
	{
	  p=nextsub-str()+1;
	  if (p > endpos)
	    break;
	  sub=nextsub;
	}
    }
  else if (from>=0 && from <= size)
    sub= strstr(str()+from, s);
  
  if (sub != NULL)
    {
      start = sub-str();
      end = start + len;
      return 1;
    }
  else
    {
      return 0;
    }

}

Declare_KVL_N(EST_String, EST_Val, 100)

template<class K, class V> EST_TKVL<K, V> &EST_TKVL<K, V>::operator = 
(const EST_TKVL<K, V> &kv)
{
    list = kv.list;
    return *this;
}



#define SWAPINT(x) ((((unsigned)x) & 0xff) << 24 | \
                    (((unsigned)x) & 0xff00) << 8 | \
		    (((unsigned)x) & 0xff0000) >> 8 | \
                    (((unsigned)x) & 0xff000000) >> 24)
#define SWAPSHORT(x) ((((unsigned)x) & 0xff) << 8 | \
                      (((unsigned)x) & 0xff00) >> 8)

#define WAVE_FORMAT_PCM    0x0001
#define WAVE_FORMAT_ADPCM  0x0002
#define WAVE_FORMAT_ALAW   0x0006
#define WAVE_FORMAT_MULAW  0x0007

void swap_bytes_short(short *data, int length)
{
    /* Swap shorts in an array */
    int i;

    for (i=0; i<length; i++)
	data[i] = SWAPSHORT(data[i]);

}

void schar_to_short(const unsigned char *chars,short *data,int length)
{
    /* Convert 8 bit data to shorts SIGNED CHAR */
    int i;

    for (i=0; i<length; i++)
	data[i] = (((unsigned char)chars[i]))*256;
    
}






template<class ENUM, class VAL, class INFO>
INFO &EST_TValuedEnumI<ENUM,VAL,INFO>::info (ENUM token) const
{
  int i;

  for(i=0; i<this->ndefinitions; i++)
    if (this->definitions[i].token == token)
      return this->definitions[i].info;

  exit(1);//cerr << "Fetching info for invalid entry\n";
  abort();

  static INFO dummyI;
  return dummyI;
}

template<class ENUM, class VAL, class INFO> 
ENUM EST_TValuedEnumI<ENUM,VAL,INFO>::token (VAL value) const
{
    int i,j;

    for(i=0; i<this->ndefinitions; i++)
	for(j=0; j<NAMED_ENUM_MAX_SYNONYMS && this->definitions[i].values[j] ; j++)
	    if (eq_vals(this->definitions[i].values[j], value))
		return this->definitions[i].token;

    return this->p_unknown_enum;
}


void EST_TokenStream::default_values()
{
    type = tst_none;
    peeked_tokp = FALSE;
    peeked_charp = FALSE;
    eof_flag = FALSE;
    quotes = FALSE;
    p_filepos = 0;
    linepos = 1;  
    WhiteSpaceChars = EST_Token_Default_WhiteSpaceChars;
    SingleCharSymbols = EST_String::Empty;
    PrePunctuationSymbols = EST_String::Empty;
    PunctuationSymbols = EST_String::Empty;
    build_table();
    close_at_end=TRUE;
}

template<class ENUM, class VAL, class INFO>
VAL EST_TValuedEnumI<ENUM,VAL,INFO>::value (ENUM token, int n) const
{
  int i;

  for(i=0; i<this->ndefinitions; i++)
    if (this->definitions[i].token == token)
      return this->definitions[i].values[n];

  return this->p_unknown_value;
}

int EST_TokenStream::seek(int position)
{
    peeked_charp = FALSE;
    peeked_tokp = FALSE;

    switch (type)
    {
      case tst_none: 
	exit(1);//cerr << "EST_TokenStream unset" << endl;
	return -1;
	break;
      case tst_file:
	p_filepos = position;
	return fseek(fp,position,SEEK_SET);
      case tst_pipe:
	exit(1);//cerr << "EST_TokenStream seek on pipe not supported" << endl;
	return -1;
	break;
      case tst_istream:
	exit(1);//cerr << "EST_TokenStream seek on istream not yet supported" << endl;
	return -1;
	break;
      case tst_string:
	if (position >= pos)
	{
	    pos = position;
	    return -1;
	}
	else
	{
	    pos = position;
	    return 0;
	}
	break;
      default:
	exit(1);//cerr << "EST_TokenStream: unknown type" << endl;
	return -1;
    }

    return -1;  // can't get here 

}

template<class ENUM, class VAL, class INFO> 
int EST_TValuedEnumI<ENUM,VAL,INFO>::n(void) const
{ 
return this->ndefinitions; 
}

EST_String::EST_String(const char *s, int start_or_fill, int len) 
{

  if (s)
    {
      int start= start_or_fill;
      if (len <0)
	len=safe_strlen(s)-start;
      
      size=len;
      if (size != 0)
	memory = chunk_allocate(len+1, s+start, len);
      else
	memory=NULL;
    }
  else
    {
      char fill = start_or_fill;
      if (len<0) len=0;
      size=len;
      if (size != 0)
	{
	  memory = chunk_allocate(len+1);
	  char *p = memory;
	  for(int j=0; j<len;j++)
	    p[j] = fill;
	  p[len]='\0';
	}
      else
	memory=NULL;
    }
}

void EST_Featured::clear_features()
{
  if (p_features)
    {
      delete p_features;
      p_features=NULL;
    }
  init_features();
}

void EST_Featured::init_features()
{
  p_features=NULL;
}

template<class T>
void EST_TVector<T>::resize(int new_cols, int set)
{
  int i;
  T * old_vals = p_memory;
  int old_cols = num_columns();
  int old_offset = p_offset;
  int old_column_step = p_column_step;

  just_resize(new_cols, &old_vals);

  if (set)
    {
      int copy_c = 0;

      if (!old_vals)
	copy_c=0;
      else if (old_vals != p_memory)
	{
	  copy_c = Lof(num_columns(), old_cols);

	  for(i=0; i<copy_c; i++)
	      a_no_check(i) 
		= old_vals[vcell_pos(i,
				    old_column_step)];
	}
      else 
	copy_c = old_cols;
      
      for(i=copy_c; i<new_cols; i++)
	  a_no_check(i) =  *def_val;
    }

  if (old_vals && old_vals != p_memory && !p_sub_matrix)
    delete [] (old_vals-old_offset);
}

template<class T>
void EST_TMatrix<T>::resize(int new_rows, int new_cols, int set)
{
  int i,j;
  T * old_vals = this->p_memory;
  int old_rows = num_rows();
  int old_cols = num_columns();
  int old_row_step = p_row_step;
  int old_offset = this->p_offset;
  int old_column_step = this->p_column_step;

  if (new_rows<0)
    new_rows = old_rows;
  if (new_cols<0)
    new_cols = old_cols;

  just_resize(new_rows, new_cols, &old_vals);

  if (set)
    {
      int copy_r = 0;
      int copy_c = 0;

      if (old_vals != NULL)
	{
	  copy_r = Lof(num_rows(), old_rows);
	  copy_c = Lof(num_columns(), old_cols);

	  set_values(old_vals,
		     old_row_step, old_column_step,
		     0, copy_r,
		     0, copy_c);
	}
      else
	{
	  copy_r = old_rows;
	  copy_c = old_cols;
	}
      
      for(i=0; i<copy_r; i++)
	for(j=copy_c; j<new_cols; j++)
	  a_no_check(i,j) =  *this->def_val;
      
      for(i=copy_r; i<new_rows; i++)
	for(j=0; j<new_cols; j++)
	  a_no_check(i,j) =  *this->def_val;
    }

  if (old_vals && old_vals != this->p_memory && !this->p_sub_matrix)
    delete [] (old_vals-old_offset);
}

// should copy from and delete old version first
template<class T> void EST_TSimpleVector<T>::resize(int newn, int set)
{
  int oldn = this->n();
  T *old_vals =NULL;
  int old_offset = this->p_offset;

  just_resize(newn, &old_vals);

  if (set && old_vals)
    {
      int copy_c = 0;
      if (this->p_memory != NULL)
	{
	  copy_c = Lof(this->n(), oldn);
	  memcpy((void *)this->p_memory, (const void *)old_vals,  copy_c* sizeof(T));
	}
      
      for (int i=copy_c; i < this->n(); ++i)
	this->p_memory[i] = *this->def_val;
    }
  
  if (old_vals != NULL && old_vals != this->p_memory && !this->p_sub_matrix)
    delete [] (old_vals - old_offset);

}

void EST_Track::copy(const EST_Track& a)
{
    copy_setup(a);
    p_values = a.p_values;
    p_times = a.p_times;
    p_is_val = a.p_is_val;
    p_t_offset = a.p_t_offset;
    p_aux = a.p_aux;
    p_aux_names = a.p_aux_names;
}

int EST_Track::val(int i) const
{
    return !p_is_val(i);
}

void uchar_to_short(const unsigned char *chars,short *data,int length)
{
    /* Convert 8 bit data to shorts  UNSIGNED CHAR */
    int i;

    for (i=0; i<length; i++)
      {
      data[i] = (((int)chars[i])-128)*256;
      }
    
}

static short st_ulaw_to_short( unsigned char ulawbyte )
{
    static int exp_lut[8] = { 0, 132, 396, 924, 1980, 4092, 8316, 16764 };
    int sign, exponent, mantissa;
    short sample;

    ulawbyte = ~ ulawbyte;
    sign = ( ulawbyte & 0x80 );
    exponent = ( ulawbyte >> 4 ) & 0x07;
    mantissa = ulawbyte & 0x0F;
    sample = exp_lut[exponent] + ( mantissa << ( exponent + 3 ) );
    if ( sign != 0 ) sample = -sample;

    return sample;
}

void ulaw_to_short(const unsigned char *ulaw,short *data,int length)
{
    /* Convert ulaw to shorts */
    int i;

    for (i=0; i<length; i++)
	data[i] = st_ulaw_to_short(ulaw[i]);  /* ulaw convert */
    
}

static int stdio_fread(void *buff,int size,int nitems,FILE *fp)
{
    // So it can find the stdio one rather than the TokenStream one
    return fread(buff,size,nitems,fp);
}


int EST_TokenStream::fread(void *buff, int size, int nitems)
{
    // switching into binary mode for current position
    int items_read;

    // so we can continue to read afterwards
    if (peeked_tokp)
    {
	exit(1);//cerr << "ERROR " << pos_description() 
	   // << " peeked into binary data" << endl;
	return 0;
    }

    peeked_charp = FALSE;
    peeked_tokp = FALSE;

    switch (type)
    {
      case tst_none: 
	exit(1);//cerr << "EST_TokenStream unset" << endl;
	return 0;
	break;
      case tst_file:
	items_read = stdio_fread(buff,(size_t)size,(size_t)nitems,fp);
	p_filepos += items_read*size;
	return items_read;
      case tst_pipe:
	exit(1);//cerr << "EST_TokenStream fread pipe not yet supported" << endl;
	return 0;
	break;
      case tst_istream:
	exit(1);//cerr << "EST_TokenStream fread istream not yet supported" << endl;
	return 0;
      case tst_string:
	if ((buffer_length-pos)/size < nitems)
	    items_read = (buffer_length-pos)/size;
	else
	    items_read = nitems;
	memcpy(buff,&buffer[pos],items_read*size);
	pos += items_read*size;
	return items_read;
      default:
	exit(1);//cerr << "EST_TokenStream: unknown type" << endl;
	return EOF;
    }

    return 0;  // can't get here 

}

template<class T>
void EST_TMatrix<T>::set_memory(T *buffer, int offset, 
				int rows, int columns, 
				int free_when_destroyed)
{
  EST_TVector<T>::set_memory(buffer, offset, columns, free_when_destroyed);
  p_num_rows = rows;
  p_row_step = columns;
}

void EST_TokenStream::build_table()
{
    int i;
    const char *p;
    unsigned char c;

    for (i=0; i<256; ++i)
	p_table[i]=0;

    for (p=WhiteSpaceChars; *p; ++p)
	if (p_table[c=(unsigned char)*p])
	    exit(1);//EST_warning("Character '%c' has two classes, '%c' and '%c'", 
		//	*p, c, ' ');
	else
	    p_table[c] = ' ';

    for (p=SingleCharSymbols; *p; ++p)
	if (p_table[c=(unsigned char)*p])
	    exit(1);//EST_warning("Character '%c' has two classes, '%c' and '%c'", 
		//	*p, p_table[c], '!');
	else
	    p_table[c] = '@';

    for (p=PunctuationSymbols; *p; ++p)
	if (p_table[c=(unsigned char)*p] == '@')
	    continue;
	else if (p_table[c])
	    exit(1);//EST_warning("Character '%c' has two classes, '%c' and '%c'", 
		//	*p, p_table[c], '.');
	else
	    p_table[c] = '.';

    for(p=PrePunctuationSymbols; *p; ++p)
	if (p_table[c=(unsigned char)*p] == '@')
	    continue;
	else if (p_table[c] == '.')
	    p_table[c] = '"';
	else if (p_table[c])
	    exit(1);//EST_warning("Character '%c' has two classes, '%c' and '%c'", 
		//	*p, p_table[c], '$');
	else
	    p_table[c] = '$';

    p_table_wrong=0;
}

const EST_String EST_String::Empty("");

const EST_String EST_Token_Default_WhiteSpaceChars = " \t\n\r";

EST_ChunkPtr chunk_allocate(int bytes)
{
  EST_Chunk *cp = new(bytes) EST_Chunk;

  return (EST_ChunkPtr)cp;
}


template<class T>
EST_TVector<T> &EST_TVector<T>::operator=(const EST_TVector<T> &in)
{
    copy(in);
    return *this;
}

template<class T>
EST_TMatrix<T> &EST_TMatrix<T>::operator=(const EST_TMatrix<T> &in)
{
    copy(in);
    return *this;
}

template<class T> EST_TSimpleVector<T> &EST_TSimpleVector<T>::operator=(const EST_TSimpleVector<T> &in)
{
    copy(in);
    return *this;
}

void EST_Track::copy_setup(const EST_Track& a)
{
    p_equal_space = a.p_equal_space;
    p_single_break = a.p_single_break;
    p_channel_names = a.p_channel_names;
    p_map = a.p_map;
    copy_features(a);
}    

template<class T> EST_TSimpleMatrix<T> &EST_TSimpleMatrix<T>::operator=(const EST_TSimpleMatrix<T> &in)
{
    copy(in);
    return *this;
}

template<class T> 
void EST_TSimpleMatrix<T>::copy(const EST_TSimpleMatrix<T> &a)
{
  if (this->num_rows() != a.num_rows() || this->num_columns() != a.num_columns())
    resize(a.num_rows(), a.num_columns(), 0);
  
  copy_data(a);
}

void EST_Featured::copy_features(const EST_Featured &f)
{
  clear_features();

  if (f.p_features)
    p_features = new EST_Features(*(f.p_features));
}

template<class T>
EST_TVector<T>::EST_TVector(int n)
{
    default_vals();
    resize(n);
}

void EST_UList::clear_and_free(void (*item_free)(EST_UItem *p))
{
    EST_UItem *p, *np;

    for (p=head(); p != 0; p = np)
    {
	np=next(p);
	if (item_free)
	    item_free(p);
	else
	    delete p;
    }
    h = t = 0;
}

template<class T> void EST_TList<T>::free_item(EST_UItem *item)
{ EST_TItem<T>::release((EST_TItem<T> *)item); }

void EST_UList::append(EST_UItem *new_item)
{

    if (new_item == 0) return;

    new_item->n = 0;
    new_item->p = t;
    if (t == 0)
	h = new_item;
    else
	t->n = new_item;
    t = new_item;
}

template<class T> EST_TItem<T> *EST_TItem<T>::make(const T &val)
{
  EST_TItem<T> *it=NULL;
  if (s_free!=NULL)
    {
      void *mem = s_free;
      s_free=(EST_TItem<T> *)s_free->n;
      s_nfree--;

      // Create an item in the retrieved memory.
      it=new (mem) EST_TItem<T>(val);
    }
  else
    it = new EST_TItem<T>(val);

  return it;
}

template<class ENUM, class VAL, class INFO>
ENUM EST_TValuedEnumI<ENUM,VAL,INFO>::nth_token (int n) const
{
  if (n>=0 && n < this->ndefinitions)
    return this->definitions[n].token;

  return this->p_unknown_enum;
}

EST_TrackMap::~EST_TrackMap()
{
}

template<class T>
void EST_TVector<T>::default_vals()
{
  p_num_columns = 0;
  p_offset=0;
  p_column_step=0;

  p_memory = NULL;
  p_sub_matrix=FALSE;
}

template<class T>
void EST_TMatrix<T>::default_vals()
{
  EST_TVector<T>::default_vals();
  p_num_rows = 0;
  p_row_step=0;
}

template<class T>
void EST_TMatrix<T>::set_values(const T *data, 
				int r_step, int c_step,
				int start_r, int num_r,
				int start_c, int num_c
				)
{
  for(int r=start_r, i=0, rp=0; i< num_r; i++, r++, rp+=r_step)
    for(int c=start_c, j=0, cp=0; j< num_c; j++, c++, cp+=c_step)
      a_no_check(r,c) = data[rp+cp];
}

template<class T>
void EST_TMatrix<T>::just_resize(int new_rows, 
				 int new_cols, 
				 T** old_vals)
{
    T *new_m;

    if (num_rows() != new_rows || num_columns() != new_cols || this->p_memory == NULL )
      {
	if (this->p_sub_matrix)
	  exit(1);//EST_error("Attempt to resize Sub-Matrix");

	if (new_cols < 0 || new_rows < 0)
	  exit(1);//EST_error("Attempt to resize matrix to negative size: %d x %d",
		//    new_rows,
		//    new_cols);

	
	new_m = new T[new_rows*new_cols];

	if (this->p_memory != NULL)
	  if (old_vals != NULL)
	    *old_vals = this->p_memory;
	  else  if (!this->p_sub_matrix)
	    delete [] (this->p_memory-this->p_offset);
    
	p_num_rows = new_rows;
	this->p_num_columns = new_cols;
	this->p_offset=0;
	p_row_step=this->p_num_columns; 
	this->p_column_step=1;
	
	this->p_memory = new_m;
      }
    else
      *old_vals = this->p_memory;
	
}

template<class T> void EST_TSimpleVector<T>::copy(const EST_TSimpleVector<T> &a)
{
  if (this->p_column_step==1 && a.p_column_step==1)
    {
    resize(a.n(), FALSE);
    memcpy((void *)(this->p_memory), (const void *)(a.p_memory), this->n() * sizeof(T));
    }
else
  ((EST_TVector<T> *)this)->copy(a);
}

template<class T>
void EST_TVector<T>::just_resize(int new_cols, T** old_vals)
{
  T *new_m;
  
  if (num_columns() != new_cols || p_memory == NULL )
    {
      if (p_sub_matrix)
	exit(1);//EST_error("Attempt to resize Sub-Vector");

      if (new_cols < 0)
	exit(1);//EST_error("Attempt to resize vector to negative size: %d",
		//  new_cols);

      new_m = new T[new_cols];

      if (p_memory != NULL)
	if (old_vals != NULL)
	  *old_vals = p_memory;
	else if (!p_sub_matrix)
	  delete [] (p_memory-p_offset);

      p_memory = new_m;
      //cout << "vr: mem: " << p_memory << " (" << (int)p_memory << ")\n";
      p_offset=0;
      p_num_columns = new_cols;
      p_column_step=1;
    }
  else
    *old_vals = p_memory;
}

template<class T>
void EST_TVector<T>::set_memory(T *buffer, int offset, int columns, 
				int free_when_destroyed)
{
  if (p_memory != NULL && !p_sub_matrix)
    delete [] (p_memory-p_offset);
  
  p_memory = buffer-offset;
  p_offset=offset;
  p_num_columns = columns;
  p_column_step=1;
  p_sub_matrix = !free_when_destroyed;
}

template<class T> 
void EST_TSimpleMatrix<T>::copy_data(const EST_TSimpleMatrix<T> &a)
{
  if (!a.p_sub_matrix && !this->p_sub_matrix)
    memcpy((void *)&this->a_no_check(0,0),
	   (const void *)&a.a_no_check(0,0),
	   this->num_rows()*this->num_columns()*sizeof(T)
	   );
  else
    {
    for (int i = 0; i < this->num_rows(); ++i)
      for (int j = 0; j < this->num_columns(); ++j)
	this->a_no_check(i,j) = a.a_no_check(i,j);
    }
}

Declare_TVector(short)

template<class T> void EST_TItem<T>::release(EST_TItem<T> *it)
{
  if (s_nfree < s_maxFree)
    {
      // Destroy the value in case it holds resources.
      it->EST_TItem<T>::~EST_TItem();

      // I suppose it's a bit weird to use 'n' after calling the destructor.
      it->n=s_free;
      s_free=it;
      s_nfree++;
    }
  else
    delete it;
}

EST_Val &EST_Val::operator=(const EST_Val &c)
{
    // Have to be careful with the case where they are different types
    if ((t != val_int) &&
	(t != val_float) &&
	(t != val_unset) &&
	(t != val_string))
	delete v.pval;
	
    if (c.t == val_string) 
	sval = c.sval;
    else if (c.t == val_int) 
	v.ival = c.v.ival;
    else if (c.t == val_float) 
	v.fval = c.v.fval;
    else if (c.t != val_unset)
    {   // does references not a real copy
	v.pval = new EST_Contents;
	*v.pval = *c.v.pval;
    }
    t=c.t; 
    return *this;
}

template<class T> EST_TList<T> &EST_TList<T>::operator=(const EST_TList<T> &a) 
{
    clear();			// clear out all current items in list.
    copy_items(a);
    return *this;
}

int operator == (const EST_String &a, const EST_String &b)
{
    if (a.size==0)
	return b.size == 0;
    else if (b.size == 0)
	return 0;
    else 
	return a.size == b.size && a(0) == b(0) && memcmp(a.str(),b.str(),a.size)==0;
};

Declare_TVector(char)
Declare_TVector(EST_Val)
Declare_TVector(EST_String)

template<class T>
void EST_TMatrix<T>::copy(const EST_TMatrix<T> &a)
{
  resize(a.num_rows(), a.num_columns(), 0);
  copy_data(a);
}

template<class T>
void EST_TVector<T>::copy(const EST_TVector<T> &a)
{
    resize(a.n(), FALSE);
    copy_data(a);
}

template<class T> void EST_TList<T>::copy_items(const EST_TList<T> &l)
{
    EST_UItem *p;
    for (p = l.head();  p; p = next(p))
	append(l.item(p));
}

template<class T>
void EST_TMatrix<T>::copy_data(const EST_TMatrix<T> &a)
{

  set_values(a.p_memory,
	     a.p_row_step, a.p_column_step,
	     0, a.num_rows(),
	     0, a.num_columns());
}

template<class T>
void EST_TVector<T>::copy_data(const EST_TVector<T> &a)
{
  set_values(a.p_memory, a.p_column_step, 0, num_columns());
}

EST_Val::EST_Val(const EST_Val &c)
{
    if (c.t == val_string) 
	sval = c.sval;
    else if (c.t == val_int) 
	v.ival = c.v.ival;
    else if (c.t == val_float) 
	v.fval = c.v.fval;
    else if (c.t != val_unset)
    {    // does references not a real copy
	v.pval = new EST_Contents;
	*v.pval = *c.v.pval;
    }
    t=c.t; 
}

template<class T>
void EST_TVector<T>::set_values(const T *data, 
				 int step,
				 int start_c, 
				 int num_c)
{
  for(int i=0, c=start_c, p=0; i<num_c; i++, c++, p+=step)
    a_no_check(c) = data[p];
}

template<class ENUM, class VAL, class INFO> 
void EST_TValuedEnumI<ENUM,VAL,INFO>::initialise(const void *vdefs)
{
  int n=0;
  typedef EST_TValuedEnumDefinition<ENUM,VAL,INFO> defn;
  const defn *defs = (const defn *)vdefs;

  for(n=1; defs[n].token != defs[0].token; n++)
    ;

  this->ndefinitions = n;
  this->definitions = new defn[n];

  this->definitions[0] = defs[0];
  for(n=1; defs[n].token != defs[0].token; n++)
    this->definitions[n] = defs[n];

  this->p_unknown_enum = defs[n].token;
  this->p_unknown_value = defs[n].values[0];
}

template<class ENUM, class VAL, class INFO>
EST_TValuedEnumI<ENUM,VAL,INFO>::~EST_TValuedEnumI(void)
{
  if (this->definitions)
     delete[] this->definitions;
}

struct Srpd_Op {
  int sample_freq;      /* Hz */
  int Nmax, Nmin;
  double shift, length; /* ms */
  double min_pitch;     /* Hz */
  double max_pitch;     /* Hz */
  int L;                /* Decimation factor (samples) */
  double Tmin, Tmax_ratio, Thigh, Tdh;
  int Tsilent;
  int make_ascii;
  int peak_tracking;
};

#define MINARG                5
#define BREAK_NUMBER          0.0

#define DEFAULT_DECIMATION    4     /* samples */
#define DEFAULT_MIN_PITCH     60.0  /* Hz */
#define DEFAULT_MAX_PITCH     600.0 /* Hz */

#define DEFAULT_SF            20000 /* Hz. Sampling Frequency */
#define DEFAULT_SHIFT         5.0   /* ms */
#define DEFAULT_LENGTH        10.0   /* ms */
#define DEFAULT_TSILENT       120   /* max. abs sample amplitude of noise */
#define DEFAULT_TMIN          0.75
#define DEFAULT_TMAX_RATIO    0.85
#define DEFAULT_THIGH         0.88
#define DEFAULT_TDH           0.77

enum Voice {
	UNVOICED = 0,
	VOICED = 1,
	SILENT = 2,
};


enum Hold {
	HOLD = 1,
	HELD = 1,
	SEND = 2,
	SENT = 2,
};


static struct Srpd_Op *default_srpd_op(struct Srpd_Op *srpd)
{ 
    srpd->L = DEFAULT_DECIMATION;
    srpd->min_pitch = DEFAULT_MIN_PITCH;
    srpd->max_pitch = DEFAULT_MAX_PITCH;
    srpd->shift = DEFAULT_SHIFT;
    srpd->length = DEFAULT_LENGTH;
    srpd->Tsilent = DEFAULT_TSILENT;
    srpd->Tmin = DEFAULT_TMIN;
    srpd->Tmax_ratio = DEFAULT_TMAX_RATIO;
    srpd->Thigh = DEFAULT_THIGH;
    srpd->Tdh = DEFAULT_TDH;
    srpd->make_ascii = 0;
    srpd->peak_tracking = 0;
    srpd->sample_freq = DEFAULT_SF;
      /* p_par->Nmax and p_par->Nmin cannot be initialised */
    return(srpd);
}

struct STATUS_
{
  double pitch_freq;
  Voice v_uv;
  Hold s_h;
  double cc_max, threshold;
};

#define rint(N) ((float)(int)((N)+0.5))

void initialise_structures (struct Srpd_Op *p_par, SEGMENT_ *p_seg, CROSS_CORR_ *p_cc)
{
  p_par->Nmax = (int) ceil((float)p_par->sample_freq / p_par->min_pitch);
  p_par->Nmin = (int) floor((float)p_par->sample_freq / p_par->max_pitch);
  p_par->min_pitch = (float)p_par->sample_freq / (float)p_par->Nmax;
  p_par->max_pitch = (float)p_par->sample_freq / (float)p_par->Nmin;

  p_seg->size = 3 * p_par->Nmax;
  p_seg->shift = (int) rint( p_par->shift / 1000.0 * (float)p_par->sample_freq );
  p_seg->length = (int) rint( p_par->length / 1000.0 * (float)p_par->sample_freq );
  p_seg->data = walloc(short,p_seg->size);

  p_cc->size = p_par->Nmax - p_par->Nmin + 1;
  p_cc->coeff = walloc(double,p_cc->size);
}

void initialise_status (const Srpd_Op &paras, STATUS_ *p_status)
{

  p_status->pitch_freq = BREAK_NUMBER;
  p_status->v_uv = SILENT;
  p_status->s_h = SEND; /* SENT */
  p_status->cc_max = 0.0;
  p_status->threshold = paras.Thigh;
  return;

}


short ConvertFloatTo16Bit( float fSample )
{
	short ret = fSample * 32768.0f;
	return ret;
}

int read_next_wave_segment2(RageSoundReader_FileReader *sample, const Srpd_Op &paras, SEGMENT_ *p_seg)
{
	printf("read:  size %d shift %d length %d\n", p_seg->size, p_seg->shift, p_seg->length);

	ASSERT( sample->GetNumChannels() == 1 );

	int iSize = p_seg->size;
	float *pfData = new float[iSize];
	int iNumRead = sample->Read( pfData, iSize );
	
	for( int i=0; i<iSize; ++i )
	{
		if( i < iNumRead )
		{
			float f = pfData[i];
			short d = ConvertFloatTo16Bit(f);
			p_seg->data[i] = d;
		}
		else
		{
			p_seg->data[i] = 0;
		}
	}

	SAFE_DELETE_ARRAY( pfData );

	return iNumRead > 0;
}


typedef struct list {
  int N0, score;
  struct list *next_item;
} LIST_;

void add_to_list (LIST_ **p_list_hd, LIST_ **p_list_tl, int N_val, 
		  int score_val)
{

  LIST_ *new_node, *last_node;

  new_node = walloc(LIST_ ,1);
  last_node = *p_list_tl;
  new_node->N0 = N_val;
  new_node->score = score_val;
  new_node->next_item = NULL;
  if (*p_list_hd == NULL)
    *p_list_hd = new_node;
  else
    last_node->next_item = new_node;
  *p_list_tl = new_node;

}

void free_list (LIST_ **p_list_hd)
{

  LIST_ *next;

  while (*p_list_hd != NULL) {
    next = (*p_list_hd)->next_item;
    wfree (*p_list_hd);
    *p_list_hd = next;
  }

}

void super_resolution_pda (const Srpd_Op &paras, const SEGMENT_ &seg, CROSS_CORR_ *p_cc, STATUS_ *p_status)
{
  static int zx_lft_N, zx_rht_N;
  static double prev_pf = BREAK_NUMBER;

  int n, j, k, N0 = 0, N1, N2, N_, q, lower_found = 0, score = 1, apply_bias;
  int x_index, y_index, z_index;
  int zx_rate = 0, zx_at_N0 = 0, prev_sign;
  int seg1_zxs = 0, seg2_zxs = 0, total_zxs;
  short prev_seg1, prev_seg2;
  short x_max = -MAXSHORT, x_min = MAXSHORT;
  short y_max = -MAXSHORT, y_min = MAXSHORT;
  double xx = 0.0, yy = 0.0, zz = 0.0, xy = 0.0, yz = 0.0, xz = 0.0;
  double max_cc = 0.0, coefficient, coeff_weight;
  double xx_N, yy_N, xy_N, y1y1_N, xy1_N, yy1_N, beta;
  LIST_ *sig_pks_hd, *sig_pks_tl, *sig_peak, *head, *tail;
  
  sig_pks_hd = head = NULL;
  sig_pks_tl = tail = NULL;
  /* set correlation coefficient threshold */
  if (p_status->v_uv == UNVOICED || p_status->v_uv == SILENT)
    p_status->threshold = paras.Thigh;
  else /* p_status->v_uv == VOICED */
    p_status->threshold = (paras.Tmin > paras.Tmax_ratio *
        p_status->cc_max) ? paras.Tmin : paras.Tmax_ratio *
	p_status->cc_max;
  /* determine if a bias should be applied */
  if (paras.peak_tracking && prev_pf != BREAK_NUMBER &&
      p_status->v_uv == VOICED && p_status->s_h != HOLD &&
      p_status->pitch_freq < 1.75 * prev_pf &&
      p_status->pitch_freq > 0.625 * prev_pf)
    apply_bias = 1;
  else
    apply_bias = 0;
  /* consider first two segments of period n = Nmin */
  prev_seg1 = seg.data[paras.Nmax - paras.Nmin] < 0 ? -1 : 1;
  prev_seg2 = seg.data[paras.Nmax] < 0 ? -1 : 1;
  for (j = 0; j < paras.Nmin; j += paras.L) {
    /* find max and min amplitudes in x and y segments */
    x_index = paras.Nmax - paras.Nmin + j;
    y_index = paras.Nmax + j;
    if (seg.data[x_index] > x_max) x_max = seg.data[x_index];
    if (seg.data[x_index] < x_min) x_min = seg.data[x_index];
    if (seg.data[y_index] > y_max) y_max = seg.data[y_index];
    if (seg.data[y_index] < y_min) y_min = seg.data[y_index];
    /* does new sample in x or y segment represent an input zero-crossing */
    if (seg.data[x_index] * prev_seg1 < 0) {
      prev_seg1 *= -1;
      seg1_zxs++;
    }
    if (seg.data[y_index] * prev_seg2 < 0) {
      prev_seg2 *= -1;
      seg2_zxs++;
    }
    /* calculate parts for first correlation coefficient */
    xx += (double) seg.data[x_index] * seg.data[x_index];
    yy += (double) seg.data[y_index] * seg.data[y_index];
    xy += (double) seg.data[x_index] * seg.data[y_index];
  }
  /* low amplitude segment represents silence */
  if (abs (x_max) + abs (x_min) < 2 * paras.Tsilent || 
      abs (y_max) + abs (y_min) < 2 * paras.Tsilent) {
    for (q = 0; q < p_cc->size; p_cc->coeff[q++] = 0.0);
    prev_pf = p_status->pitch_freq;
    p_status->pitch_freq = BREAK_NUMBER;
    p_status->v_uv = SILENT;
    p_status->s_h = SEND;
    p_status->cc_max = 0.0;
    return;
  }
  /* determine first correlation coefficients, for period n = Nmin */
  p_cc->coeff[0] = p_status->cc_max = xy / sqrt (xx) / sqrt (yy);
  for (q = 1; q < p_cc->size && q < paras.L; p_cc->coeff[q++] = 0.0);
  total_zxs = seg1_zxs + seg2_zxs;
  prev_sign = p_cc->coeff[0] < 0.0 ? -1 : 1;
  prev_seg1 = seg.data[paras.Nmax - paras.Nmin] < 0 ? -1 : 1;
  /* iteratively determine correlation coefficient for next possible period */
  for (n = paras.Nmin + paras.L; n <= paras.Nmax; n += paras.L,
       j += paras.L) {
    x_index = paras.Nmax - n;
    y_index = paras.Nmax + j;
    /* does new samples in x or y segment represent an input zero-crossing */
    if (seg.data[x_index] * prev_seg1 < 0) {
      prev_seg1 *= -1;
      total_zxs++;
    }
    if (seg.data[y_index] * prev_seg2 < 0) {
      prev_seg2 *= -1;
      total_zxs++;
    }
    /* determine next coefficient */
    xx += (double) seg.data[x_index] * seg.data[x_index];
    yy += (double) seg.data[y_index] * seg.data[y_index];
    for (k = 0, xy = 0.0; k < n; k += paras.L)
      xy += (double) seg.data[paras.Nmax - n + k] * seg.data[paras.Nmax + k];
    p_cc->coeff[n - paras.Nmin] = xy / sqrt (xx) / sqrt (yy);
    if (p_cc->coeff[n - paras.Nmin] > p_status->cc_max)
      p_status->cc_max = p_cc->coeff[n - paras.Nmin];
    /* set unknown coefficients to zero */
    for (q = n - paras.Nmin + 1;
	 q < p_cc->size && q < n - paras.Nmin + paras.L;
	 p_cc->coeff[q++] = 0.0);
    /* is there a slope with positive gradient in the coefficients track yet */
    if (p_cc->coeff[n - paras.Nmin] > p_cc->coeff[n - paras.Nmin - paras.L])
      lower_found = 1;
    /* has new coefficient resulted in a zero-crossing */
    if (p_cc->coeff[n - paras.Nmin] * prev_sign < 0.0) {
      prev_sign *= -1;
      zx_rate++;
    }
    /* does the new coefficient represent a pitch period candidate */
    if (N0 != 0 && zx_rate > zx_at_N0) {
      add_to_list (&sig_pks_hd, &sig_pks_tl, N0, 1);
      N0 = 0;
      max_cc = 0.0;
    }
    if (apply_bias && n > zx_lft_N && n < zx_rht_N)
      coeff_weight = 2.0;
    else
      coeff_weight = 1.0;
    if (p_cc->coeff[n - paras.Nmin] > max_cc && total_zxs > 3 && lower_found) {
      max_cc = p_cc->coeff[n - paras.Nmin];
      if (max_cc * coeff_weight >= p_status->threshold) {
	zx_at_N0 = zx_rate;
	N0 = n;
      }
    }
  }
  /* unvoiced if no significant peak found in coefficients track */
  if (sig_pks_hd == NULL) {
    prev_pf = p_status->pitch_freq;
    p_status->pitch_freq = BREAK_NUMBER;
    p_status->v_uv = UNVOICED;
    p_status->s_h = SEND;
    return;
  }
  /* find which significant peak in list corresponds to true pitch period */
  sig_peak = sig_pks_hd;
  while (sig_peak != NULL) {
    yy = zz = yz = 0.0;
    for (j = 0; j < sig_peak->N0; j++) {
      y_index = paras.Nmax + j;
      z_index = paras.Nmax + sig_peak->N0 + j;
      yy += (double) seg.data[y_index] * seg.data[y_index];
      zz += (double) seg.data[z_index] * seg.data[z_index];
      yz += (double) seg.data[y_index] * seg.data[z_index];
    }
    if (yy == 0.0 || zz == 0.0)
      coefficient = 0.0;
    else
      coefficient = yz / sqrt (yy) / sqrt (zz);
    if (apply_bias && sig_peak->N0 > zx_lft_N && sig_peak->N0 < zx_rht_N)
      coeff_weight = 2.0;
    else
      coeff_weight = 1.0;
    if (coefficient * coeff_weight >= p_status->threshold) {
      sig_peak->score = 2;
      if (head == NULL) {
	head = sig_peak;
	score = 2;
      }
      tail = sig_peak;
    }
    sig_peak = sig_peak->next_item;
  }
  if (head == NULL) head = sig_pks_hd;
  if (tail == NULL) tail = sig_pks_tl;
  N0 = head->N0;
  if (tail != head) {
    xx = 0.0;
    for (j = 0; j < tail->N0; j++)
      xx += (double) seg.data[paras.Nmax - tail->N0 + j] *
	  seg.data[paras.Nmax - tail->N0 + j];
    sig_peak = head;
    while (sig_peak != NULL) {
      if (sig_peak->score == score) {
	xz = zz = 0.0;
	for (j = 0; j < tail->N0; j++) {
	  z_index = paras.Nmax + sig_peak->N0 + j;
	  xz += (double) seg.data[paras.Nmax - tail->N0 + j] *
	      seg.data[z_index];
	  zz += (double) seg.data[z_index] * seg.data[z_index];
	}
	coefficient = xz / sqrt (xx) / sqrt (zz);
	if (sig_peak == head)
	  max_cc = coefficient;
	else if (coefficient * paras.Tdh > max_cc) {
	  N0 = sig_peak->N0;
	  max_cc = coefficient;
	}
      }
      sig_peak = sig_peak->next_item;
    }
  }
  p_status->cc_max = p_cc->coeff[N0 - paras.Nmin];
  /* voiced segment period now found */
  if ((tail == head && score == 1 && p_status->v_uv != VOICED) ||
      p_cc->coeff[N0 - paras.Nmin] < p_status->threshold)
    p_status->s_h = HOLD;
  else
    p_status->s_h = SEND;
  /* find left and right boundaries of peak in coefficients track */
  zx_lft_N = zx_rht_N = 0;
  for (q = N0; q >= paras.Nmin; q -= paras.L)
    if (p_cc->coeff[q - paras.Nmin] < 0.0) {
      zx_lft_N = q;
      break;
    }
  for (q = N0; q <= paras.Nmax; q += paras.L)
    if (p_cc->coeff[q - paras.Nmin] < 0.0) {
      zx_rht_N = q;
      break;
    }
  /* define small region around peak */
  if (N0 - paras.L < paras.Nmin) {
    N1 = N0;
    N2 = N0 + 2 * paras.L;
  }
  else if (N0 + paras.L > paras.Nmax) {
    N1 = N0 - 2 * paras.L;
    N2 = N0;
  }
  else {
    N1 = N0 - paras.L;
    N2 = N0 + paras.L;
  }
  /* compensate for decimation factor L */
  if (paras.L != 1) {
    xx = yy = xy = 0.0;
    for (j = 0; j < N1; j++) {
      x_index = paras.Nmax - N1 + j;
      y_index = paras.Nmax + j;
      xx += (double) seg.data[x_index] * seg.data[x_index];
      xy += (double) seg.data[x_index] * seg.data[y_index];
      yy += (double) seg.data[y_index] * seg.data[y_index];
    }
    p_cc->coeff[N1 - paras.Nmin] = p_status->cc_max =
        xy / sqrt (xx) / sqrt (yy);
    N0 = N1;
    for (n = N1 + 1; n <= N2; n++, j++) {
      xx += (double) seg.data[paras.Nmax - n] * seg.data[paras.Nmax - n];
      yy += (double) seg.data[paras.Nmax + j] * seg.data[paras.Nmax + j];
      for (k = 0, xy = 0.0; k < n; k++)
	xy += (double) seg.data[paras.Nmax - n + k] * seg.data[paras.Nmax + k];
      p_cc->coeff[n - paras.Nmin] = xy / sqrt (xx) / sqrt (yy);
      if (p_cc->coeff[n - paras.Nmin] > p_status->cc_max) {
	p_status->cc_max = p_cc->coeff[n - paras.Nmin];
	N0 = n;
      }
    }
  }
  /* compensate for finite resolution in estimating pitch */
  if (N0 - 1 < paras.Nmin || N0 == N1) N_ = N0;
  else if (N0 + 1 > paras.Nmax || N0 == N2) N_ = N0 - 1;
  else if (p_cc->coeff[N0 - paras.Nmin] - p_cc->coeff[N0 - paras.Nmin - 1] <
	   p_cc->coeff[N0 - paras.Nmin] - p_cc->coeff[N0 - paras.Nmin + 1])
    N_ = N0 - 1;
  else
    N_ = N0;
  xx_N = yy_N = xy_N = y1y1_N = xy1_N = yy1_N = 0.0;
  for (j = 0; j < N_; j++) {
    x_index = paras.Nmax - N_ + j;
    y_index = paras.Nmax + j;
    xx_N += (double) seg.data[x_index] * seg.data[x_index];
    yy_N += (double) seg.data[y_index] * seg.data[y_index];
    xy_N += (double) seg.data[x_index] * seg.data[y_index];
    y1y1_N += (double) seg.data[y_index + 1] * seg.data[y_index + 1];
    xy1_N += (double) seg.data[x_index] * seg.data[y_index + 1];
    yy1_N += (double) seg.data[y_index] * seg.data[y_index + 1];
  }
  beta = (xy1_N * yy_N - xy_N * yy1_N) /
      (xy1_N * (yy_N - yy1_N) + xy_N * (y1y1_N - yy1_N));
  if (beta < 0.0) {
    N_--;
    beta = 0.0;
  }
  else if (beta >= 1.0) {
    N_++;
    beta = 0.0;
  }
  else
    p_status->cc_max = ((1.0 - beta) * xy_N + beta * xy1_N) /
      sqrt (xx_N * ((1.0 - beta) * (1.0 - beta) * yy_N +
		    2.0 * beta * (1.0 - beta) * yy1_N +
		    beta * beta * y1y1_N));
  prev_pf = p_status->pitch_freq;
  p_status->pitch_freq = (double) (paras.sample_freq) / (double) (N_ + beta);
  p_status->v_uv = VOICED;
  free_list (&sig_pks_hd);
  return;

}



DetectPitch::DetectPitch()
{
	m_pSrpdOp = new Srpd_Op;
	m_pSegment = new SEGMENT_;
	m_iSamplesFilledInSegment = 0;
	m_pCC = new CROSS_CORR_;
	m_pPdaStatus = new STATUS_;
}

DetectPitch::~DetectPitch()
{
	SAFE_DELETE( m_pSrpdOp );
	SAFE_DELETE( m_pSegment );
	m_iSamplesFilledInSegment = 0;
	SAFE_DELETE( m_pCC );
	SAFE_DELETE( m_pPdaStatus );
}

void DetectPitch::Init(int iSampleFreq)
{
	default_srpd_op(m_pSrpdOp); // default values

	m_pSrpdOp->sample_freq = iSampleFreq;
	initialise_structures (m_pSrpdOp, m_pSegment, m_pCC);
	for( int i=0; i<m_pCC->size; ++i )
		m_pCC->coeff[i] = 0.0;
	initialise_status (*m_pSrpdOp, m_pPdaStatus);

	// add a low-pass filter?
}

int DetectPitch::ReadOne(RageSoundReader_FileReader *sample)
{
	if( read_next_wave_segment2 (sample, *m_pSrpdOp, m_pSegment) != 0 ) 
	{
		super_resolution_pda (*m_pSrpdOp, *m_pSegment, m_pCC, m_pPdaStatus);

		printf( "track set:  (of %d) is %f - voiced %d, held %d\n", m_pSegment->length, m_pPdaStatus->pitch_freq, m_pPdaStatus->v_uv, m_pPdaStatus->s_h );
		return true;
	}

	end_structure_use (m_pSegment, m_pCC);
	return false;
}

int DetectPitch::ReadOne(short *pData, int iCount)
{
	//printf("read:  size %d shift %d length %d\n", m_pSegment->size, m_pSegment->shift, m_pSegment->length);

	for( int iDataReadCount = 0; iDataReadCount < iCount; )
	{
		int iSamplesLeftInSegment = m_pSegment->size - m_iSamplesFilledInSegment;
		int iSamplesLeftInData = iCount - iDataReadCount;
		int iSamplesToCopy = min( iSamplesLeftInSegment, iSamplesLeftInData );

		memcpy( m_pSegment->data + m_iSamplesFilledInSegment, pData + iDataReadCount, iSamplesToCopy*sizeof(short) );

		m_iSamplesFilledInSegment += iSamplesToCopy;
		iDataReadCount += iSamplesToCopy;

		ASSERT( m_iSamplesFilledInSegment <= m_pSegment->size );
		if( m_iSamplesFilledInSegment == m_pSegment->size )	// segment is full
		{
			super_resolution_pda (*m_pSrpdOp, *m_pSegment, m_pCC, m_pPdaStatus);
			//printf( "track set:  (of %d) is %f - voiced %d, held %d\n", m_pSegment->length, m_pPdaStatus->pitch_freq, m_pPdaStatus->v_uv, m_pPdaStatus->s_h );
			//printf( "v %d, h %d ", m_pPdaStatus->v_uv, m_pPdaStatus->s_h );
			const int iNumBars = 30;
			for( int i=0; i<iNumBars; i++ )
			{
				//const int freq = SCALE( i, 0, iNumBars, 1, 400 );
				//printf( (m_pPdaStatus->pitch_freq < freq) ? "|" : "X" );
			}
			//printf( "\n" );

			int iNumSamplesToShift = m_pSegment->size - m_pSegment->shift;
			m_iSamplesFilledInSegment = iNumSamplesToShift;
			memcpy( m_pSegment->data, m_pSegment->data + m_pSegment->shift, iNumSamplesToShift*sizeof(short) );
		}

	}
	return true;
}

void DetectPitch::GetStatus( MicrophoneStatus &out )
{
	out.fFreq = m_pPdaStatus->pitch_freq;
	out.fMaxFreq = m_pSrpdOp->max_pitch;
	out.bVoiced = m_pPdaStatus->v_uv == VOICED;
}

void DetectPitch::End()
{
	end_structure_use (m_pSegment, m_pCC);
}


void srpd2()
{
	RageFile file;
	file.Open( "speech-test.wav" );
	RageSoundReader_FileReader *sample = new RageSoundReader_WAV;
	sample->Open( &file );

	DetectPitch dp;
	dp.Init( SAMPLES_PER_SEC );
	while( dp.ReadOne(sample) )
	{

	}
}



/*
 * (c) 2007 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
