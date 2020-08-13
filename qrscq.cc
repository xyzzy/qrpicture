/*
 *  This file is part of qrpicture, photo realistic QR-codes.
 *  Copyright (C) 2007, xyzzy@rockingship.net
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Based on the original work of: 
 *  Copyright (c) 2006 Derrick Coetzee
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FILTER
#define FILTER 5
#endif

unsigned char imgQR[93*93];
double gsavg;

#define TOGRAY_SRGB(R,G,B) ((R)*0.212656 + (G)*0.715158 + (B)*0.072186)
#define TOGRAY_PAL(R,G,B)  ((R)*0.299    + (G)*0.587    + (B)*0.114   )
#define TO_GRAY(R,G,B) TOGRAY_SRGB(R,G,B)

static int seqnr = 0;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <gd.h>
#include <sys/times.h>

#include <string>

int verbose = 0;
double opt_thresh = 0.22; // 0.26;
int opt_filter = 1;
int opt_hksk = 0;
int opt_palette_size;

int hksk[99][3] = {
{0,0,0},
{255,255,255},
{246,233,143},
{244,239,27},
{251,243,21},
{239,213,34},
{232,196,37},
{223,168,39},
{214,138,39},
{211,127,39},
{201,77,38},
{225,174,148},
{199,63,38},
{196,24,42},
{195,25,36},
{191,25,58},
{160,37,49},
{161,35,71},
{125,44,62},
{220,154,166},
{200,63,56},
{196,18,69},
{186,19,91},
{198,0,114},
{175,10,119},
{192,0,134},
{174,22,101},
{142,11,135},
{174,0,135},
{186,0,135},
{135,13,136},
{104,20,136},
{71,45,145},
{89,47,145},
{88,22,137},
{13,44,90},
{70,113,186},
{35,132,185},
{22,45,98},
{51,53,148},
{49,78,163},
{42,107,183},
{91,86,166},
{80,138,195},
{11,161,126},
{15,161,213},
{29,142,199},
{126,190,230},
{28,161,169},
{35,160,146},
{102,177,140},
{40,149,96},
{27,128,95},
{49,116,89},
{38,139,75},
{93,126,84},
{26,140,133},
{103,170,67},
{44,77,13},
{131,134,66},
{106,140,56},
{97,163,63},
{138,188,63},
{149,192,61},
{160,197,59},
{218,203,41},
{229,231,35},
{212,173,42},
{204,144,42},
{175,133,24},
{139,100,12},
{140,124,54},
{77,46,3},
{99,62,43},
{133,54,43},
{191,89,39},
{157,61,22},
{128,65,49},
{106,47,36},
{24,23,21},
{188,183,155},
{59,50,40},
{173,168,168},
{124,127,126},
{77,85,85},
{152,152,131},
{140,138,124},
{55,59,46},
{133,115,77},
{161,160,159},
{-1,-1,-1},
};

void logline(const char *fmt, ...)
{
	struct tms tms;
	times(&tms);

	static int ticks_per_sec = 0;
	if (!ticks_per_sec) 
		ticks_per_sec = sysconf(_SC_CLK_TCK);

	static clock_t t0, tlast;
	clock_t t = tms.tms_utime;

	if (tlast == 0)
		t0 = tlast = t;

	fprintf(stderr, "%7.3f(%7.3f) ", (t-t0)*1.0/ticks_per_sec, (t-tlast)*1.0/ticks_per_sec);
	tlast = t;

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

using namespace std;

class Pixel
{
public:
	Pixel()
	{
		r = g = b = 0;
	}

	Pixel(double r, double g, double b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}

	Pixel(const Pixel& rhs)
	{
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
	}

	Pixel& operator=(const Pixel rhs)
	{
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		return *this;
	}

	Pixel& operator=(const double scalar)
	{
		r = scalar;
		g = scalar;
		b = scalar;
		return *this;
	}

	Pixel& operator+=(Pixel rhs) {
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		return *this;
	}

	Pixel& operator*=(double scalar) {
		r *= scalar;
		g *= scalar;
		b *= scalar;
		return *this;
	}

	Pixel operator*(double scalar) {
		return Pixel(this->r * scalar, this->g * scalar, this->b * scalar);
	}

public:
	double r,g,b;
};

template <typename T>
class array2d
{
public:
    array2d(int width, int height)
    {
        this->width = width;
        this->height = height;

	this->ymul = this->width + 2;
	this->ofs  = 2 * this->ymul + 2;

	data = new T[(this->height + 2) * this->ymul + this->ofs];

	memset(this->data, 0, sizeof(T[(this->height + 2) * this->ymul + this->ofs]));
    }

    array2d(const array2d<T>& rhs)
    {
        width = rhs.width;
        height = rhs.height;

	this->ymul = (this->width + 2) * 1;
	this->ofs  = 2 * this->ymul + 2 * 1;

	data = new T[(this->height + 2) * this->ymul + this->ofs];

	memcpy(this->data, rhs.data, sizeof(T[(this->height + 2) * this->ymul + this->ofs]));
    }

    ~array2d()
    {
	delete [] data;
    }

    T& operator()(int col, int row)
    {
	return data[row*ymul + col + ofs];
    }

    int get_width() { return width; }
    int get_height() { return height; }

    array2d<T>& operator*=(T scalar) {
        for(int i=0; i<width; i++) {
	    for(int j=0; j<height; j++) {
		(*this)(i,j) *= scalar;
	    }
	}
	return *this;
    }

    array2d<T> operator*(T scalar) {
	array2d<T> result(*this);
	result = result*scalar;
	return result;
    }

private:
    T* data;
    int width, height;
    int ymul, ofs;
};

template <typename T>
array2d<T> operator*(T scalar, array2d<T> a) {
    return a*scalar;
}

class CoarseVariables
{
public:
	CoarseVariables(int width, int height, int depth) {
		this->width = width;
		this->height = height;
		this->depth = depth;

		this->xmul = depth;
		this->ymul = (this->width + 2) * this->xmul;
		this->ofs  = 2 * this->ymul + 2 * this->xmul;

		data = new double[(this->height + 2) * this->ymul + this->ofs];
	}

	~CoarseVariables() {
		delete[] data;
	}

	int get_width() {
		return width;
	}
	int get_height() {
		return height;
	}
	int get_depth() {
		return depth;
	}

	double& operator()(int x, int y, int z) {
		return data[y*ymul + x*xmul + z + ofs];
	}

	void clear_border()
	{
		for (int i=0; i<ofs; i++) {
			data[i] = 0;
			data[(this->height + 2) * this->ymul + i] = 0;
		}
		for (int y=0; y<height; y++) {
			for (int x=0; x<2*xmul; x++) {
				data[y*ymul+ofs-x-1] = 0;
			}
		}
	}

public:
	double *data;
	int width, height, depth;
	int ymul, xmul, ofs;
};

void compute_a_image(array2d< Pixel >& image,
                     double b00, double b01, double b02, double b11, double b12, double b22,
                     array2d< Pixel >& a)
{
	const int ofs00 = &image(-2,-2) - &image(0,0) ;
	const int ofs01 = &image(-1,-2) - &image(0,0) ;
	const int ofs02 = &image( 0,-2) - &image(0,0) ;
	const int ofs03 = &image(+1,-2) - &image(0,0) ;
	const int ofs04 = &image(+2,-2) - &image(0,0) ;
	const int ofs10 = &image(-2,-1) - &image(0,0) ;
	const int ofs11 = &image(-1,-1) - &image(0,0) ;
	const int ofs12 = &image( 0,-1) - &image(0,0) ;
	const int ofs13 = &image(+1,-1) - &image(0,0) ;
	const int ofs14 = &image(+2,-1) - &image(0,0) ;
	const int ofs20 = &image(-2, 0) - &image(0,0) ;
	const int ofs21 = &image(-1, 0) - &image(0,0) ;
	const int ofs22 = &image( 0, 0) - &image(0,0) ;
	const int ofs23 = &image(+1, 0) - &image(0,0) ;
	const int ofs24 = &image(+2, 0) - &image(0,0) ;
	const int ofs30 = &image(-2,+1) - &image(0,0) ;
	const int ofs31 = &image(-1,+1) - &image(0,0) ;
	const int ofs32 = &image( 0,+1) - &image(0,0) ;
	const int ofs33 = &image(+1,+1) - &image(0,0) ;
	const int ofs34 = &image(+2,+1) - &image(0,0) ;
	const int ofs40 = &image(-2,+2) - &image(0,0) ;
	const int ofs41 = &image(-1,+2) - &image(0,0) ;
	const int ofs42 = &image( 0,+2) - &image(0,0) ;
	const int ofs43 = &image(+1,+2) - &image(0,0) ;
	const int ofs44 = &image(+2,+2) - &image(0,0) ;

    for(int i_y = 0; i_y < a.get_height(); i_y++) {
	for(int i_x = 0; i_x < a.get_width(); i_x++) {

		Pixel *pdata = &image(i_x,i_y);

		    Pixel sum;
		    sum += pdata[ofs00] * b00;
		    sum += pdata[ofs01] * b01;
		    sum += pdata[ofs02] * b02;
		    sum += pdata[ofs03] * b01;
		    sum += pdata[ofs04] * b00;

		    sum += pdata[ofs10] * b01;
		    sum += pdata[ofs11] * b11;
		    sum += pdata[ofs12] * b12;
		    sum += pdata[ofs13] * b11;
		    sum += pdata[ofs14] * b01;

		    sum += pdata[ofs20] * b02;
		    sum += pdata[ofs21] * b12;
		    sum += pdata[ofs22] * b22;
		    sum += pdata[ofs23] * b12;
		    sum += pdata[ofs24] * b02;

		    sum += pdata[ofs30] * b01;
		    sum += pdata[ofs31] * b11;
		    sum += pdata[ofs32] * b12;
		    sum += pdata[ofs33] * b11;
		    sum += pdata[ofs34] * b01;

		    sum += pdata[ofs40] * b00;
		    sum += pdata[ofs41] * b01;
		    sum += pdata[ofs42] * b02;
		    sum += pdata[ofs43] * b01;
		    sum += pdata[ofs44] * b00;

		    a(i_x,i_y)  = sum;
	}
    }
}

void compute_a_image_3(array2d< Pixel >& image,
                     double b11, double b12, double b22,
                     array2d< Pixel >& a)
{
	const int ofs11 = &image(-1,-1) - &image(0,0) ;
	const int ofs12 = &image( 0,-1) - &image(0,0) ;
	const int ofs13 = &image(+1,-1) - &image(0,0) ;
	const int ofs21 = &image(-1, 0) - &image(0,0) ;
	const int ofs22 = &image( 0, 0) - &image(0,0) ;
	const int ofs23 = &image(+1, 0) - &image(0,0) ;
	const int ofs31 = &image(-1,+1) - &image(0,0) ;
	const int ofs32 = &image( 0,+1) - &image(0,0) ;
	const int ofs33 = &image(+1,+1) - &image(0,0) ;

    for(int i_y = 0; i_y < a.get_height(); i_y++) {
	for(int i_x = 0; i_x < a.get_width(); i_x++) {

		Pixel *pdata = &image(i_x,i_y);

		    Pixel sum;
		    sum += pdata[ofs11] * b11;
		    sum += pdata[ofs12] * b12;
		    sum += pdata[ofs13] * b11;

		    sum += pdata[ofs21] * b12;
		    sum += pdata[ofs22] * b22;
		    sum += pdata[ofs23] * b12;

		    sum += pdata[ofs31] * b11;
		    sum += pdata[ofs32] * b12;
		    sum += pdata[ofs33] * b11;

		    a(i_x,i_y)  = sum;
	}
    }
}

int best_match_color(CoarseVariables &vars, int i_x, int i_y)
{
    int max_v = 0;
    double max_weight = vars(i_x, i_y, 0);
    for (int v=1; v < opt_palette_size; v++) {
	if (vars(i_x, i_y, v) > max_weight) {
	    max_v = v;
	    max_weight = vars(i_x, i_y, v);
	}
    }
    return max_v;
}

void refine_palette(array2d< double >& __s,
		    Pixel *__r,
		    Pixel *palette)
{
if (opt_hksk) {
	double threshlow =  0.5-opt_thresh ;
	double threshhigh = 0.5+opt_thresh ;
	if (gsavg-opt_thresh < threshlow) threshlow = gsavg-opt_thresh;
	if (gsavg+opt_thresh > threshhigh) threshhigh = gsavg+opt_thresh;

	opt_palette_size = 0;
	for (int i=0; hksk[i][0] >= 0; i++) {
		double r = hksk[i][0] / 255.0;
		double g = hksk[i][1] / 255.0;
		double b = hksk[i][2] / 255.0;
		double gs = TO_GRAY(r,g,b);

		if (gs <= threshlow || gs >= threshhigh) {
			palette[opt_palette_size++] = Pixel(r,g,b);
		}
	}
	return;
}

//palette_size = 3;
//__s(0,0) = 1;
//__s(0,1) = 1.5;
//__s(0,2) = 2;
//__s(1,0) = 2;
//__s(1,1) = 3;
//__s(1,2) = 4;
//__s(2,0) = 3;
//__s(2,1) = 6;
//__s(2,2) = 9;
//printf("%f %f %f\n", __s(0,0), __s(0,1), __s(0,2));
//printf("%f %f %f\n", __s(1,0), __s(1,1), __s(1,2));
//printf("%f %f %f\n", __s(2,0), __s(2,1), __s(2,2));

    // We only computed the half of S above the diagonal - reflect it
    for (int v=0; v<opt_palette_size; v++) {
	for (int alpha=0; alpha<v; alpha++) {
	    __s(v,alpha) = __s(alpha,v);
	}
    }
//printf("%f %f %f\n", __s(0,0), __s(0,1), __s(0,2));
//printf("%f %f %f\n", __s(1,0), __s(1,1), __s(1,2));
//printf("%f %f %f\n", __s(2,0), __s(2,1), __s(2,2));

	// rewrite: vector<double> palette_channel = -1.0*((2.0*S_k).matrix_inverse())*R_k;

        double source[opt_palette_size*opt_palette_size];
        double result[opt_palette_size*opt_palette_size];

	if (1) {
		// Set result to identity matrix
		for(int i=0; i<opt_palette_size; i++) {
			for(int j=0; j<opt_palette_size; j++) {
				source[j*opt_palette_size+i] = __s(i,j);
				result[j*opt_palette_size+i] = 0;
			}
			result[i*opt_palette_size+i] = 1;
		}

		// Reduce to echelon form, mirroring in result
		for(int i=0; i<opt_palette_size; i++) {
			double mult = source[i*opt_palette_size+i];
			for(int k=0; k<opt_palette_size; k++) result[i*opt_palette_size+k] = result[i*opt_palette_size+k] / mult;
			for(int k=0; k<opt_palette_size; k++) source[i*opt_palette_size+k] = source[i*opt_palette_size+k] / mult;
			for(int j=i+1; j<opt_palette_size; j++) {
				double mult = source[j*opt_palette_size+i];
				for(int k=0; k<opt_palette_size; k++) result[j*opt_palette_size+k] -= result[i*opt_palette_size+k] * mult;
				for(int k=0; k<opt_palette_size; k++) source[j*opt_palette_size+k] -= source[i*opt_palette_size+k] * mult;
			}
		}
		// Back substitute, mirroring in result
		for(int i=opt_palette_size-1; i>=0; i--) {
			for(int j=i-1; j>=0; j--) {
				double mult = source[j*opt_palette_size+i];
				for(int k=0; k<opt_palette_size; k++) result[j*opt_palette_size+k] -= result[i*opt_palette_size+k] * mult;
				for(int k=0; k<opt_palette_size; k++) source[j*opt_palette_size+k] -= source[i*opt_palette_size+k] * mult;
			}
		}
	}
//printf("%f %f %f\n", result[0], result[1], result[2]);
//printf("%f %f %f\n", result[3], result[4], result[5]);
//printf("%f %f %f\n", result[6], result[7], result[8]);

	// keep black.white
	for(int row=0; row<opt_palette_size; row++) {
		Pixel sum;
		for(int col=0; col<opt_palette_size; col++)
			sum += __r[col] * result[row*opt_palette_size+col];

		Pixel val = sum;
val.r = round(val.r*255)/255.0;
val.g = round(val.g*255)/255.0;
val.b = round(val.b*255)/255.0;
		double gs = TO_GRAY(val.r, val.g, val.b);

double threshlow =  0.5-opt_thresh ;
double threshhigh = 0.5+opt_thresh ;
if (gsavg-opt_thresh < threshlow) threshlow = gsavg-opt_thresh;
if (gsavg+opt_thresh > threshhigh) threshhigh = gsavg+opt_thresh;
if (1) {
		if (gs > threshlow && gs < threshhigh) {
			if (gs < 0.5) {
				val.r = val.r * (threshlow)/gs;
				val.g = val.g * (threshlow)/gs;
				val.b = val.b * (threshlow)/gs;
			} else {
				val.r = 1-((1-val.r) * (1-threshhigh)/(1-gs));
				val.g = 1-((1-val.g) * (1-threshhigh)/(1-gs));
				val.b = 1-((1-val.b) * (1-threshhigh)/(1-gs));

			}
		}
		gs = TO_GRAY(val.r, val.g, val.b);
		if (gs > threshlow+0.00001 && gs < threshhigh-0.00001) {
			fprintf(stderr, "#1 %f %f %f\n", threshlow, gs, threshhigh);
//			exit(0);
		}
}

//if (val.r < 0 || val.g < 0 || val.b < 0) fprintf(stderr,"### %f %f %f\n", val.r, val.g, val.b);
//if (val.r > 1 || val.g > 1 || val.b > 1) fprintf(stderr,"!!! %f %f %f\n", val.r, val.g, val.b);
		if (val.r <= 0) val.r = 0; //!! NOT DETECTED
		if (val.r > 1) val.r = 1; //!! DETECTED
		if (val.g <= 0) val.g = 0; //!! NOT DETECTED
		if (val.g > 1) val.g = 1; //!! DETECTED
		if (val.b <= 0) val.b = 0; //!! NOT DETECTED
		if (val.b > 1) val.b = 1; //!! DETECTED
		palette[row] = val;
	}
}

void spatial_color_quant(array2d< Pixel >& image,
                         array2d< int >& quantized_image,
                         Pixel *palette,
			 double initial_temperature,
			 double final_temperature,
			 int temps_per_level,
			 int repeats_per_temp,
			 int fixed_palette,
			 int rng_seed)
{
//double filter00 = 1.0/16.0;
//double filter01 = 2.0/16.0;
//double filter11 = 4.0/16.0;

//--
const int fil1_00 = 0;
const int fil1_01 = 0;
const int fil1_11 = 16;
//--
const int fil3_00 = 1;
const int fil3_01 = 2;
const int fil3_11 = 4;
//--
const int fil5_00 =  1; //   filter00*filter00;
const int fil5_01 =  4; // 2*filter00*filter01;
const int fil5_02 =  6; // 2*filter00*filter00 + filter01*filter01;
const int fil5_11 = 16; // 2*filter00*filter11 + 2*filter01*filter01;
const int fil5_12 = 24; // 4*filter00*filter01 + 2*filter01*filter11;
const int fil5_22 = 36; // 4*filter00*filter00 + 4*filter01*filter01 + filter11*filter11;
//--

    int coarse_level = 0;
    int image_width = image.get_width();
    int image_height = image.get_height();

    array2d< Pixel > a0(image_width, image_height);
    CoarseVariables *p_coarse_variables = new CoarseVariables(image_width  >> coarse_level, image_height >> coarse_level, opt_palette_size);
    array2d< double > __s(opt_palette_size, opt_palette_size);
    Pixel palette0[opt_palette_size];
    int palette_count[opt_palette_size];
    int palette_shade[opt_palette_size];

			    double ming, maxg; 
			    int minv=-1, maxv=-1;
			    for (int i=0; i<opt_palette_size; i++) {
				double gs = TO_GRAY(palette[i].r, palette[i].g, palette[i].b);
				if (minv<0 || gs < ming) { ming=gs; minv=i; }
				if (maxv<0 || gs > maxg) { maxg=gs; maxv=i; }
				palette_shade[i] = (gs < 0.5) ? 1 : 2;
			    }

//---
	const int fofs1_00 = &(*p_coarse_variables)(-1,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs1_01 = &(*p_coarse_variables)( 0,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs1_02 = &(*p_coarse_variables)(+1,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs1_10 = &(*p_coarse_variables)(-1, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs1_11 = &(*p_coarse_variables)( 0, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs1_12 = &(*p_coarse_variables)(+1, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs1_20 = &(*p_coarse_variables)(-1,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs1_21 = &(*p_coarse_variables)( 0,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs1_22 = &(*p_coarse_variables)(+1,+1,0) - &(*p_coarse_variables)(0,0,0) ;
//---
	const int fofs3_00 = &(*p_coarse_variables)(-1,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs3_01 = &(*p_coarse_variables)( 0,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs3_02 = &(*p_coarse_variables)(+1,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs3_10 = &(*p_coarse_variables)(-1, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs3_11 = &(*p_coarse_variables)( 0, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs3_12 = &(*p_coarse_variables)(+1, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs3_20 = &(*p_coarse_variables)(-1,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs3_21 = &(*p_coarse_variables)( 0,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs3_22 = &(*p_coarse_variables)(+1,+1,0) - &(*p_coarse_variables)(0,0,0) ;
//---
	const int fofs5_00 = &(*p_coarse_variables)(-2,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_01 = &(*p_coarse_variables)(-1,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_02 = &(*p_coarse_variables)( 0,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_03 = &(*p_coarse_variables)(+1,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_04 = &(*p_coarse_variables)(+2,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_10 = &(*p_coarse_variables)(-2,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_11 = &(*p_coarse_variables)(-1,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_12 = &(*p_coarse_variables)( 0,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_13 = &(*p_coarse_variables)(+1,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_14 = &(*p_coarse_variables)(+2,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_20 = &(*p_coarse_variables)(-2, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_21 = &(*p_coarse_variables)(-1, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_22 = &(*p_coarse_variables)( 0, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_23 = &(*p_coarse_variables)(+1, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_24 = &(*p_coarse_variables)(+2, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_30 = &(*p_coarse_variables)(-2,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_31 = &(*p_coarse_variables)(-1,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_32 = &(*p_coarse_variables)( 0,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_33 = &(*p_coarse_variables)(+1,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_34 = &(*p_coarse_variables)(+2,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_40 = &(*p_coarse_variables)(-2,+2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_41 = &(*p_coarse_variables)(-1,+2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_42 = &(*p_coarse_variables)( 0,+2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_43 = &(*p_coarse_variables)(+1,+2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int fofs5_44 = &(*p_coarse_variables)(+2,+2,0) - &(*p_coarse_variables)(0,0,0) ;
//---

    // Compute a_i, b_{ij} according to (11)

    // linear random generator with values of a,c,m as used in glibc
    int lcg = rng_seed;

if (opt_filter==1) {
    compute_a_image_3(image, fil1_00/16.0, fil1_01/16.0, fil1_11/16.0, a0);
} else if (opt_filter==3) {
    compute_a_image_3(image, fil3_00/16.0, fil3_01/16.0, fil3_11/16.0, a0);
} else if (opt_filter==5) {
    compute_a_image(image, fil5_00/256.0,fil5_01/256.0,fil5_02/256.0,fil5_11/256.0,fil5_12/256.0,fil5_22/256.0, a0);
}

    for(int j=0; j<image_height; j++) {
        for(int i=0; i<image_width; i++) {
            for(int k=0; k<p_coarse_variables->get_depth(); k++) {
		lcg = lcg * 1103515245 + 12345;
                (*p_coarse_variables)(i,j,k) = (lcg&0x7fffffff) / (double)0x7fffffff;
(*p_coarse_variables)(i,j,k) = 1.0 / opt_palette_size;
	    }
	}
    }
    p_coarse_variables->clear_border();

    for (int iters_at_current_level=0; iters_at_current_level<temps_per_level; iters_at_current_level++) {

	int coarse_width = p_coarse_variables->get_width();
	int coarse_height = p_coarse_variables->get_height();
	int coarse_size2d = coarse_width*coarse_height;
	int something_changed = 0;

	    logline("Level:%d coarseW:%4d coarseH:%4d iter:%d\n", coarse_level, coarse_width,  coarse_height, iters_at_current_level);

		for (int alpha=0; alpha<opt_palette_size; alpha++) palette0[alpha] = palette[alpha];
		if (!fixed_palette && iters_at_current_level>0) {
			Pixel __r[opt_palette_size];

			// compute initial s
			for (int alpha=0; alpha<opt_palette_size; alpha++) for (int v=0; v<=alpha; v++) __s(v,alpha) = 0;
			for (int i_y=0; i_y<coarse_height; i_y++) {
			for (int i_x=0; i_x<coarse_width; i_x++) {
			for (int a=0; a<opt_palette_size; a++) {

				double *pdata = &(*p_coarse_variables)(i_x,i_y,a);

				double sum = 0;
if (opt_filter==1) {
				sum += (pdata[fofs1_00] + pdata[fofs1_02] +  pdata[fofs1_20] +  pdata[fofs1_22]) * fil1_00;
				sum += (pdata[fofs1_01] + pdata[fofs1_10] +  pdata[fofs1_12] +  pdata[fofs1_21]) * fil1_01;
				sum /= 16.0;
				for (int v=0; v<=a; v++)
					__s(v,a) += (*p_coarse_variables)(i_x,i_y,v) * sum;
				__s(a,a) += pdata[fofs1_11] * fil1_11/16.0;
				__r[a] += a0(i_x,i_y) * pdata[fofs1_11];
} else if (opt_filter==3) {
				sum += (pdata[fofs3_00] + pdata[fofs3_02] +  pdata[fofs3_20] +  pdata[fofs3_22]) * fil3_00;
				sum += (pdata[fofs3_01] + pdata[fofs3_10] +  pdata[fofs3_12] +  pdata[fofs3_21]) * fil3_01;
				sum /= 16.0;
				for (int v=0; v<=a; v++)
					__s(v,a) += (*p_coarse_variables)(i_x,i_y,v) * sum;
				__s(a,a) += pdata[fofs3_11] * fil3_11/16.0;
				__r[a] += a0(i_x,i_y) * pdata[fofs3_11];
} else {
				sum += (pdata[fofs5_00] + pdata[fofs5_04] +  pdata[fofs5_40] +  pdata[fofs5_44]) * fil5_00;
				sum += (pdata[fofs5_01] + pdata[fofs5_10] +  pdata[fofs5_03] +  pdata[fofs5_30]+
					pdata[fofs5_14] + pdata[fofs5_41] +  pdata[fofs5_34] +  pdata[fofs5_43]) * fil5_01;
				sum += (pdata[fofs5_02] + pdata[fofs5_20] +  pdata[fofs5_24] +  pdata[fofs5_42]) * fil5_02;
				sum += (pdata[fofs5_11] + pdata[fofs5_13] +  pdata[fofs5_31] +  pdata[fofs5_33]) * fil5_11;
				sum += (pdata[fofs5_12] + pdata[fofs5_21] +  pdata[fofs5_32] +  pdata[fofs5_23]) * fil5_12;
				sum /= 256.0;
				for (int v=0; v<=a; v++)
					__s(v,a) += (*p_coarse_variables)(i_x,i_y,v) * sum;
				__s(a,a) += pdata[fofs5_22] * fil5_22/256.0;
				__r[a] += a0(i_x,i_y) * pdata[fofs5_22];
}


			}}}

			// calculate average image grayscale
			gsavg = 0;
			    for(int i_x = 0; i_x < image_width; i_x++) {
				for(int i_y = 0; i_y < image_height; i_y++) {
				   int c = best_match_color(*p_coarse_variables, i_x, i_y);
				   gsavg += TO_GRAY(palette[c].r, palette[c].g, palette[c].b);
				}
			    }
			gsavg /= image_width*image_height;
			logline("gsavg: %f\n", gsavg);

			// refine palette using s
			refine_palette(__s, __r, palette);

			    minv=-1;
			    maxv=-1;
			    for (int i=0; i<opt_palette_size; i++) {
				double gs = TO_GRAY(palette[i].r, palette[i].g, palette[i].b);
				if (minv<0 || gs < ming) { ming=gs; minv=i; }
				if (maxv<0 || gs > maxg) { maxg=gs; maxv=i; }
				palette_shade[i] = (gs < 0.5) ? 1 : 2;
			    }

			logline("updated palette\n");
		}

	    int pixels_changed = 0, pixels_visited = 0;

		char *dirty_flag = new char[coarse_size2d];
		int *dirty_xy = new int[coarse_size2d];
		int dirty_in = 0, dirty_out = 0, dirty_cnt = 0, dirty_step = 0;

		// visit all pixels
		for (dirty_in=0; dirty_in<coarse_size2d; dirty_in++) {
			dirty_flag[dirty_in] = 1;
			dirty_xy[dirty_in] = dirty_in;
		}
		dirty_cnt = coarse_size2d;
 		dirty_in %= coarse_size2d;
		dirty_step = dirty_cnt;

		// randomize
		for (int i=1; i<coarse_size2d; i++) {
			lcg = lcg * 1103515245 + 12345;
			int j =  (lcg&0x7fffffff) % (i + 1);

			int swap = dirty_xy[i];
			dirty_xy[i] = dirty_xy[j];
			dirty_xy[j] = swap;
		}

	    // Compute 2*sum(j in extended neighborhood of i, j != i) b_ij

int maxloop = 100;
	    while(dirty_cnt)
	    {
			const int i_x = dirty_xy[dirty_out]%coarse_width;
			const int i_y = dirty_xy[dirty_out]/coarse_width;
			dirty_out = (dirty_out+1)%coarse_size2d;
			dirty_cnt--;

		// Compute (25)
		Pixel p_i;

			for (int v=0; v < opt_palette_size; v++) {

				double *pdata = &(*p_coarse_variables)(i_x,i_y,v);

				double sum = 0;

if (opt_filter==1) {
				sum += (pdata[fofs1_00] + pdata[fofs1_02] +  pdata[fofs1_20] +  pdata[fofs1_22]) * fil1_00;
				sum += (pdata[fofs1_01] + pdata[fofs1_10] +  pdata[fofs1_12] +  pdata[fofs1_21]) * fil1_01;
				sum /= 16.0;
} else if (opt_filter==3) {
				sum += (pdata[fofs3_00] + pdata[fofs3_02] +  pdata[fofs3_20] +  pdata[fofs3_22]) * fil3_00;
				sum += (pdata[fofs3_01] + pdata[fofs3_10] +  pdata[fofs3_12] +  pdata[fofs3_21]) * fil3_01;
				sum /= 16.0;
} else {
				sum += (pdata[fofs5_00] + pdata[fofs5_04] +  pdata[fofs5_40] +  pdata[fofs5_44]) * fil5_00;
				sum += (pdata[fofs5_01] + pdata[fofs5_10] +  pdata[fofs5_03] +  pdata[fofs5_30]+
					pdata[fofs5_14] + pdata[fofs5_41] +  pdata[fofs5_34] +  pdata[fofs5_43]) * fil5_01;
				sum += (pdata[fofs5_02] + pdata[fofs5_20] +  pdata[fofs5_24] +  pdata[fofs5_42]) * fil5_02;
				sum += (pdata[fofs5_11] + pdata[fofs5_13] +  pdata[fofs5_31] +  pdata[fofs5_33]) * fil5_11;
				sum += (pdata[fofs5_12] + pdata[fofs5_21] +  pdata[fofs5_32] +  pdata[fofs5_23]) * fil5_12;
				sum /= 256.0;
}

				p_i += palette[v] * sum;
			}

		double meanfield_logs[opt_palette_size];
		double max_meanfield_log = 0;
		double meanfield_sum = 0.0;
		double meanfields[opt_palette_size];

		for (int v=0; v < opt_palette_size; v++) {

if (opt_filter==1) {
		    meanfield_logs[v]= ( (palette[v].r*fil1_11/2/16.0 + p_i.r - a0(i_x, i_y).r)*palette[v].r +
					 (palette[v].g*fil1_11/2/16.0 + p_i.g - a0(i_x, i_y).g)*palette[v].g +
					 (palette[v].b*fil1_11/2/16.0 + p_i.b - a0(i_x, i_y).b)*palette[v].b ) * (-2 / 0.00001);
} else if (opt_filter==3) {
		    meanfield_logs[v]= ( (palette[v].r*fil3_11/2/16.0 + p_i.r - a0(i_x, i_y).r)*palette[v].r +
					 (palette[v].g*fil3_11/2/16.0 + p_i.g - a0(i_x, i_y).g)*palette[v].g +
					 (palette[v].b*fil3_11/2/16.0 + p_i.b - a0(i_x, i_y).b)*palette[v].b ) * (-2 / 0.00001);
} else {
		    meanfield_logs[v]= ( (palette[v].r*fil5_22/2/256.0 + p_i.r - a0(i_x, i_y).r)*palette[v].r +
					 (palette[v].g*fil5_22/2/256.0 + p_i.g - a0(i_x, i_y).g)*palette[v].g +
					 (palette[v].b*fil5_22/2/256.0 + p_i.b - a0(i_x, i_y).b)*palette[v].b ) * (-2 / 0.00001);
}

		    if (v==0 || meanfield_logs[v] > max_meanfield_log)
			max_meanfield_log = meanfield_logs[v];
		}

		for (int v=0; v < opt_palette_size; v++) {
		    meanfields[v] = exp(meanfield_logs[v]-max_meanfield_log+100);

if (1) {
			if ( imgQR[(i_y>>1)*93+(i_x>>1)] == 0) {
				// dark
				if (palette_shade[v]&2)
					    meanfields[v] = 0;
			} else {
				// light
				if (palette_shade[v]&1)
					    meanfields[v] = 0;
			}
		    meanfield_sum += meanfields[v];
		}
}
//--------------
		if (meanfield_sum == 0) {
		for (int v=0; v < opt_palette_size; v++) {

if (opt_filter==1) {
		    meanfield_logs[v]= ( (palette[v].r*fil1_11/2/16.0 + p_i.r - a0(i_x, i_y).r)*palette[v].r +
					 (palette[v].g*fil1_11/2/16.0 + p_i.g - a0(i_x, i_y).g)*palette[v].g +
					 (palette[v].b*fil1_11/2/16.0 + p_i.b - a0(i_x, i_y).b)*palette[v].b ) * (-2 / 0.01);
} else if (opt_filter==3) {
		    meanfield_logs[v]= ( (palette[v].r*fil3_11/2/16.0 + p_i.r - a0(i_x, i_y).r)*palette[v].r +
					 (palette[v].g*fil3_11/2/16.0 + p_i.g - a0(i_x, i_y).g)*palette[v].g +
					 (palette[v].b*fil3_11/2/16.0 + p_i.b - a0(i_x, i_y).b)*palette[v].b ) * (-2 / 0.01);
} else {
		    meanfield_logs[v]= ( (palette[v].r*fil5_22/2/256.0 + p_i.r - a0(i_x, i_y).r)*palette[v].r +
					 (palette[v].g*fil5_22/2/256.0 + p_i.g - a0(i_x, i_y).g)*palette[v].g +
					 (palette[v].b*fil5_22/2/256.0 + p_i.b - a0(i_x, i_y).b)*palette[v].b ) * (-2 / 0.01);
}

		    if (v==0 || meanfield_logs[v] > max_meanfield_log)
			max_meanfield_log = meanfield_logs[v];
		}

		for (int v=0; v < opt_palette_size; v++) {
		    meanfields[v] = exp(meanfield_logs[v]-max_meanfield_log+100);

if (1) {
			if ( imgQR[(i_y>>1)*93+(i_x>>1)] == 0) {
				// dark
				if (palette_shade[v]&2)
					    meanfields[v] = 0;
			} else {
				// light
				if (palette_shade[v]&1)
					    meanfields[v] = 0;
			}
}
		    meanfield_sum += meanfields[v];
		}
		}
		if (meanfield_sum == 0) {
			printf("meanfield_sum==0\n");
			exit(0);
		}
//-----------------

		int old_max_v = -1;
		double old_max_weight = -1;
		int new_max_v = -1;
		double new_max_weight = -1;

		for (int v=0; v < opt_palette_size; v++) {
		    double new_val = meanfields[v]/meanfield_sum;
		    double old_val = (*p_coarse_variables)(i_x,i_y,v);
if (new_val < 0.00001) new_val = 0.00001;
if (new_val > 0.99999) new_val = 0.99999;

		    (*p_coarse_variables)(i_x,i_y,v) = new_val;

		    // update best match
		    if (old_val > old_max_weight) {
			old_max_v = v;
			old_max_weight = old_val;
		    }


		    // update best match
		    if (new_val > new_max_weight) {
			new_max_v = v;
			new_max_weight = new_val;
		    }
		}

		if (old_max_v != new_max_v) {
		    pixels_changed++;
		    something_changed++;
		    // We don't add the outer layer of pixels , because
		    // there isn't much weight there, and if it does need
		    // to be visited, it'll probably be added when we visit
		    // neighboring pixels.
if (opt_filter==3) {
		    for (int y=0; y<3; y++) {
			for (int x=0; x<3; x++) {
			    int j_x = x - 1 + i_x, j_y = y - 1 + i_y;
			    if (j_x < 0 || j_y < 0 || j_x >= coarse_width || j_y >= coarse_height) continue;

				if (!dirty_flag[j_y*coarse_width+j_x]) {
					dirty_flag[j_y*coarse_width+j_x] = 1;
					dirty_cnt++;
					dirty_xy[dirty_in] = j_y*coarse_width+j_x;
					dirty_in = (dirty_in+1)%coarse_size2d;
				}
			}
		    }
} else if (opt_filter==5) {
		    for (int y=0; y<5; y++) {
			for (int x=0; x<5; x++) {
			    int j_x = x - 2 + i_x, j_y = y - 2 + i_y;
			    if (j_x < 0 || j_y < 0 || j_x >= coarse_width || j_y >= coarse_height) continue;

				if (!dirty_flag[j_y*coarse_width+j_x]) {
					dirty_flag[j_y*coarse_width+j_x] = 1;
					dirty_cnt++;
					dirty_xy[dirty_in] = j_y*coarse_width+j_x;
					dirty_in = (dirty_in+1)%coarse_size2d;
				}
			}
		    }
}
		}

		dirty_flag[i_y*coarse_width+i_x] = 0;

		pixels_visited++;
		if (--dirty_step <= 0) {
			logline("pixels visited:%7d changed:%7d\n", pixels_visited, pixels_changed);
			dirty_step = dirty_cnt;
			pixels_visited = pixels_changed = 0;
if (--maxloop < 0) break;
		}
	    }

		delete[] dirty_flag;
		delete[] dirty_xy;

		if (verbose > 1) {
			char fname[128];
			sprintf(fname, "tmp-%02d-%d-%d-%d.png", seqnr++, coarse_level, coarse_width, coarse_height);

			for (int i=0; i<opt_palette_size; i++)
				palette_count[i] = 0;

			FILE *fil = fopen(fname, "wb");
			if (fil) {
				gdImagePtr im = gdImageCreateTrueColor(coarse_width, coarse_height);

				for(int y=0; y<coarse_height; y++) {
					for (int x=0; x<coarse_width; x++) {

						int max_v = 0;
						double max_weight = (*p_coarse_variables)(x, y, 0);
						for (int v=1; v < opt_palette_size; v++) {
							if ((*p_coarse_variables)(x, y, v) > max_weight) {
								max_v = v;
								max_weight = (*p_coarse_variables)(x, y, v);
							}
						}

						palette_count[max_v]++;
						int r = (unsigned char)(255*palette[max_v].r);
						int g = (unsigned char)(255*palette[max_v].g);
						int b = (unsigned char)(255*palette[max_v].b);
						int c = gdImageColorAllocate(im, r, g, b);
						gdImageSetPixel(im, x, y, c);
					}
				}
				gdImagePng(im, fil);
				gdImageDestroy(im);
			}
			fclose(fil);
			if (verbose > 2) 
				for (int v=0; v<opt_palette_size; v++)
					printf("%f %f %f %9d %8.5f %8.5f %8.5f\n", palette[v].r, palette[v].g, palette[v].b, palette_count[v], palette[v].r-palette0[v].r, palette[v].g-palette0[v].g, palette[v].b-palette0[v].b);
		}

		if (!something_changed)
			break;
    }

    for(int i_x = 0; i_x < image_width; i_x++) {
	for(int i_y = 0; i_y < image_height; i_y++) {
	    int c = best_match_color(*p_coarse_variables, i_x, i_y);
	    quantized_image(i_x,i_y) = c;
	}
    }
}

void usage(const char *argv0, int verbose)
{
	printf("Usage: %s [options] <source image> <mask image> <desired palette size> <output image>\n", argv0);
	if (verbose)
		return;

	printf("\nsource image is PNG/GIF/JPG\noutput image is PNG\n");
	
	printf("\n\
options:\n\
	-x		x position in source\n\
	-y		y position in source\n\
	-w n		\n\
	--width=n	width of source\n\
	-h n		\n\
	--height	height of source\n\
	-f n		\n\
	--filtersize n	filter size (1/3/5)\n\
	-d n		\n\
	-s n		\n\
	--seed=n	starting seed of random generator\n\
	-v		\n\
	--verbose	show progress\n\
\n\
	--initial-temperature=n		initial annealing temperature\n\
	--final-temperature=n		final annealing temperature\n\
	--temperature-per-level=n	number of temperature changes per level\n\
	--repeat-per-temperature=n	rescan per temperature change\n\
	--palette=file			use this color palette\n");
}

typedef struct oct_node_t oct_node_t, *oct_node;
struct oct_node_t{
	int r, g, b;
	int count, heap_idx;
	oct_node kids[8], parent;
	unsigned char n_kids, kid_idx, inheap, depth;
};
typedef struct {
	int alloc, n;
	oct_node* buf;
} node_heap;
static oct_node pool = 0;

oct_node node_new(int kid_idx, int depth, oct_node p)
{
	static int len = 0;
	if (len <= 1) {
		oct_node p = (oct_node)calloc(sizeof(oct_node_t), 2048);
		p->parent = pool;
		pool = p;
		len = 2047;
	}
 
	oct_node x = pool + len--;
	x->kid_idx = kid_idx;
	x->depth = depth;
	x->parent = p;
	if (p) p->n_kids++;
	return x;
}
oct_node node_insert(oct_node root, int R, int G, int B)
{
	for (int bit=1<<7, depth=1; depth<8; bit>>=1, depth++) {
		int i = !!(G & bit) * 4 + !!(R & bit) * 2 + !!(B & bit);
		if (!root->kids[i])
			root->kids[i] = node_new(i, depth, root);
 
		root = root->kids[i];
	}
 
	root->r += R;
	root->g += G;
	root->b += B;
	root->count++;
	return root;
}
int cmp_node(oct_node a, oct_node b)
{
	if (a->n_kids < b->n_kids) return -1;
	if (a->n_kids > b->n_kids) return 1;
 
	int ac = a->count * (1 + a->kid_idx) >> a->depth;
	int bc = b->count * (1 + b->kid_idx) >> b->depth;
	return ac < bc ? -1 : ac > bc;
}
oct_node node_fold(oct_node p)
{
	if (p->n_kids) abort();
	oct_node q = p->parent;
	q->count += p->count;
 
	q->r += p->r;
	q->g += p->g;
	q->b += p->b;
	q->n_kids --;
	q->kids[p->kid_idx] = 0;
	return q;
}

void down_heap(node_heap *h, oct_node p)
{
	int n = p->heap_idx, m;
	while (1) {
		m = n * 2;
		if (m >= h->n) break;
		if (m + 1 < h->n && cmp_node(h->buf[m], h->buf[m + 1]) > 0) m++;
 
		if (cmp_node(p, h->buf[m]) <= 0) break;
 
		h->buf[n] = h->buf[m];
		h->buf[n]->heap_idx = n;
		n = m;
	}
	h->buf[n] = p;
	p->heap_idx = n;
}
void up_heap(node_heap *h, oct_node p)
{
	int n = p->heap_idx;
	oct_node prev;
 
	while (n > 1) {
		prev = h->buf[n / 2];
		if (cmp_node(p, prev) >= 0) break;
 
		h->buf[n] = prev;
		prev->heap_idx = n;
		n /= 2;
	}

	p->heap_idx = n;
	h->buf[n] = p;
}
void heap_add(node_heap *h, oct_node p)
{
	if ((p->inheap)) {
		down_heap(h, p);
		up_heap(h, p);
		return;
	}
 
	p->inheap = 1;
	if (!h->n) h->n = 1;
	if (h->n >= h->alloc) {
		while (h->n >= h->alloc) h->alloc += 1024;
		h->buf = (oct_node*)realloc(h->buf, sizeof(oct_node) * h->alloc);
	}
 
	p->heap_idx = h->n;
	h->buf[h->n++] = p;
	up_heap(h, p);
}
 
oct_node pop_heap(node_heap *h)
{
	if (h->n <= 1) return 0;
 
	oct_node ret = h->buf[1];
	h->buf[1] = h->buf[--h->n];
 
	h->buf[h->n] = 0;
 
	h->buf[1]->heap_idx = 1;
	down_heap(h, h->buf[1]);
 
	return ret;
}
 

int main(int argc, char* argv[]) {

double opt_initial_temperature = 1.000;
double opt_final_temperature = 0.001;
int opt_tpl = 2;
int opt_rpt = 1;
int opt_verbose = 0;
int opt_width = 0;
int opt_height = 0;
int opt_seed = 0;
char *opt_source;
char *opt_output;
char *opt_mask;
const char *opt_palette = NULL;

gdImagePtr im = NULL;
FILE *fil;

	for (;;) {
		int option_index = 0;
		enum {	LO_HELP=1, LO_INITIAL, LO_FINAL, LO_TPL, LO_RPT, LO_PALETTE,
			LO_VERBOSE='v', LO_W='w', LO_H='h', LO_SEED='s', LO_FILTER='f', LO_THRESH='t' };
		static struct option long_options[] = {
			/* name, has_arg, flag, val */
			{"help", 0, 0, LO_HELP},
			{"verbose", 0, 0, LO_VERBOSE},
			{"width", 1, 0, LO_W},
			{"height", 1, 0, LO_H},
			{"seed", 1, 0, LO_SEED},
			{"filter", 1, 0, LO_FILTER},
			{"initial-temperature", 1, 0, LO_INITIAL},
			{"final-temperature", 1, 0, LO_FINAL},
			{"temperature-per-level", 1, 0, LO_TPL},
			{"repeat-per-temperature", 1, 0, LO_RPT},
			{"colourthresh", 1, 0, LO_THRESH},
			{"palette", 1, 0, LO_PALETTE},
			{NULL, 0, 0, 0}
		};

		char optstring[128], *cp;
		cp = optstring;
		for (int i=0; long_options[i].name; i++) {
			if (isalpha(long_options[i].val)) {
				*cp++ = long_options[i].val;
				if (long_options[i].has_arg)
					*cp++ = ':';
			}
		}
		*cp++ = '\0';

		int c = getopt_long (argc, argv, optstring, long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case LO_INITIAL:
			opt_initial_temperature = strtod(optarg, NULL);
			break;
		case LO_FINAL:
			opt_final_temperature = strtod(optarg, NULL);
			break;
		case LO_TPL:
			opt_tpl = strtol(optarg, NULL, 10);
			break;
		case LO_RPT:
			opt_rpt = strtol(optarg, NULL, 10);
			break;
		case LO_PALETTE:
			opt_palette = optarg;
			break;

		case LO_VERBOSE:
			opt_verbose++;
			break;
		case LO_W:
			opt_width = strtol(optarg, NULL, 10);
			break;
		case LO_H:
			opt_height = strtol(optarg, NULL, 10);
			break;
		case LO_SEED:
			opt_seed = strtol(optarg, NULL, 10);
			break;
		case LO_FILTER:
			opt_filter = strtol(optarg, NULL, 10);
			if (opt_filter != 1 && opt_filter != 3 && opt_filter != 5) {
				fprintf(stderr, "Filter size must be one of 1, 3, or 5.\n");
				return -1;
			}
			break;
		case LO_THRESH:
			opt_thresh = strtod(optarg, NULL);
			if (opt_thresh < 0 || opt_thresh >= 0.5) {
				printf("coilor threshold must be in range 0 .. 0.5\n");
				return -1;
			}
			break;
		case LO_HELP:
			usage(argv[0], 0);
			exit(0);
			break;
		case '?':
			fprintf(stderr,"Try `%s --help' for more information.\n", argv[0]);
			exit(1);
			break;
		default:
			fprintf (stderr, "getopt returned character code %d\n", c);
			exit(1);
		}
	 }

	if (argc-optind < 4) {
		usage(argv[0], 1);
		exit(1);
	}
	opt_source = argv[optind++];
	opt_mask = argv[optind++];
	opt_palette_size = strtol(argv[optind++], NULL, 10);
	opt_output = argv[optind++];

	if (opt_palette_size <= 1) {
		fprintf(stderr, "Number of colors must be at least 2\n");
		return -1;
	}

	verbose = opt_verbose;

	/* set generator to (un)known state */
	if (!opt_seed)
		opt_seed = time(NULL);
	srand(opt_seed);

//--------
	fil = fopen(opt_mask, "rb");
	if (fil == NULL) {
		fprintf(stderr, "Could not open source mask\n");
		return -1;
	}
	unsigned char c[2];
	if (fread(c, 2, 1, fil) == 1) {
		rewind(fil);
		if (c[0]==0x89 && c[1]==0x50)
			im = gdImageCreateFromPng(fil);
		if (c[0]==0x47 && c[1]==0x49)
			im = gdImageCreateFromGif(fil);
		if (c[0]==0xff && c[1]==0xd8)
			im = gdImageCreateFromJpeg(fil);
	}
	if (im == NULL) {
		fprintf(stderr, "Could not load source image %x %x\n", c[0], c[1]);
		return -1;
	}
	fclose(fil);
	for (int y=0; y<93; y++) {
		for (int x=0; x<93; x++) {
			int v = gdImageGetTrueColorPixel(im, x*2+12, y*2+12);
			int r = ((v>>16) & 0xFF) / 255.0;
			int g = ((v>>8) & 0xFF) / 255.0;
			int b = (v & 0xFF) / 255.0;
			double gs = TO_GRAY(r,g,b);
			imgQR[y*93+x] = (gs<0.5) ? 0 : 1;
		}
	}

//--------

	/* open source */
	fil = fopen(opt_source, "rb");
	if (fil == NULL) {
		fprintf(stderr, "Could not open source image\n");
		return -1;
	}
	if (fread(c, 2, 1, fil) == 1) {
		rewind(fil);
		if (c[0]==0x89 && c[1]==0x50)
			im = gdImageCreateFromPng(fil);
		if (c[0]==0x47 && c[1]==0x49)
			im = gdImageCreateFromGif(fil);
		if (c[0]==0xff && c[1]==0xd8)
			im = gdImageCreateFromJpeg(fil);
	}
	if (im == NULL) {
		fprintf(stderr, "Could not load source image %x %x\n", c[0], c[1]);
		return -1;
	}
	fclose(fil);

opt_width = opt_height = 186;

	const int width = opt_width ? opt_width : gdImageSX(im);
	const int height = opt_height ? opt_height : gdImageSY(im);
	if (width <= 0 || height <= 0) {
		fprintf(stderr, "Must specify a valid positive image width and height with --width and --height.\n");
		return -1;
	}

	Pixel palette[1024];
	if (opt_palette && strcmp(opt_palette, "octree") != 0) {
		int k = 0;
		FILE* in = fopen(opt_palette, "r");
		if (in == NULL) {
			fprintf(stderr, "Could not open palette file\n");
			return -1;
		}
		double r,g,b;
		while (fscanf(in, "%lf %lf %lf\n", &r,  &g, &b) == 3) {
			if (r > 1) r /= 255.0;
			if (g > 1) g /= 255.0;
			if (b > 1) b /= 255.0;
			if (k+1 < 1024)
				palette[k++] = Pixel(r,g,b);
		}
		fclose(in);

		if (!opt_palette_size)
			opt_palette_size = k;
	} else {
		for (int i=0; i<opt_palette_size; i++) {
			Pixel p;
			p.r = ((double)rand())/RAND_MAX;
			p.g = ((double)rand())/RAND_MAX;
			p.b = ((double)rand())/RAND_MAX;
			palette[i] = p;
		}
	}

	fprintf(stderr,"invocation: \"%s\" %d \"%s\" --width=%d --height=%d --seed=%d --filter=%d --initial-temperature=%f --final-temperature=%f --temperature-per-level=%d --repeat-per-temperature=%d --palette=\"%s\"\n",
		opt_source,opt_palette_size,opt_output,width,height,opt_seed,opt_filter,
		opt_initial_temperature,opt_final_temperature,opt_tpl,opt_rpt,
		opt_palette?opt_palette:"");

	array2d< Pixel > image(width, height);
	array2d< int > quantized_image(width, height);

//--------
	if (opt_palette_size > 2 && opt_palette && strcmp(opt_palette, "octree") == 0) {
		node_heap heap = { 0, 0, 0 };
		oct_node root = node_new(0, 0, 0);
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int v = gdImageGetTrueColorPixel(im, x, y);
				int r = ((v>>16) & 0xFF);
				int g = ((v>>8) & 0xFF);
				int b = (v & 0xFF);
				heap_add(&heap, node_insert(root, r,g,b));
			}
		}
 
		while (heap.n > opt_palette_size + 1)
			heap_add(&heap, node_fold(pop_heap(&heap)));
 
		for (int i = 1; i < heap.n; i++) {
			oct_node got = heap.buf[i];
			double c = got->count;
			got->r = got->r / c + .5;
			got->g = got->g / c + .5;
			got->b = got->b / c + .5;
			printf("%2d | %3u %3u %3u (%d pixels)\n",	i, got->r, got->g, got->b, got->count);
			palette[i-1].r = got->r / 255.0;
			palette[i-1].g = got->g / 255.0;
			palette[i-1].b = got->b / 255.0;
		}
	} else {
		palette[opt_palette_size-2].r = 0;
		palette[opt_palette_size-2].g = 0;
		palette[opt_palette_size-2].b = 0;
		palette[opt_palette_size-1].r = 1;
		palette[opt_palette_size-1].g = 1;
		palette[opt_palette_size-1].b = 1;
	}
//--------

	/* load source */
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int v = gdImageGetTrueColorPixel(im, x, y);
double r =  ((v>>16) & 0xFF) / 255.0L;
double g =  ((v>>8) & 0xFF) / 255.0L;  
double b =  (v & 0xFF) / 255.0L;
//r = pow(r,.75);
//g = pow(g,.75);
//b = pow(b,.75);
if (r>1) r = 1;
if (g>1) g = 1;
if (b>1) b = 1;
			image(x,y).r = r;
			image(x,y).g = g;
			image(x,y).b = b;
		}
	}
	gdImageDestroy(im);
	logline("source read\n");

	spatial_color_quant(image, quantized_image, palette, opt_initial_temperature, opt_final_temperature,opt_tpl,opt_rpt, opt_palette!=NULL, opt_seed);

	logline("converted\n");

//-------
// skip until found why this breaks the QR
#if 1
	fil = fopen("qrscq-mask-93x93.png", "rb");
	if (fil == NULL) {
		fprintf(stderr, "Could not open mask-scq.png\n");
		return -1;
	}
	if (fread(c, 2, 1, fil) == 1) {
		rewind(fil);
		if (c[0]==0x89 && c[1]==0x50)
			im = gdImageCreateFromPng(fil);
		if (c[0]==0x47 && c[1]==0x49)
			im = gdImageCreateFromGif(fil);
		if (c[0]==0xff && c[1]==0xd8)
			im = gdImageCreateFromJpeg(fil);
	}
	if (im == NULL) {
		fprintf(stderr, "Could not load source image %x %x\n", c[0], c[1]);
		return -1;
	}
	fclose(fil);
	for (int y=0; y<93; y++) {
		for (int x=0; x<93; x++) {
			int v = gdImageGetTrueColorPixel(im, x, y);
		 	      float a = ((v>>24) & 0xFF) / 127.0;
	 	 	      float r = ((v>>16) & 0xFF) / 255.0;
	 	 	      float g = ((v>>8) & 0xFF) / 255.0;
	 	 	      float b = (v & 0xFF) / 255.0;

	 	 	      if (a > 0.5)
	 	 	 	     imgQR[y*93+x] = 0; // mask
	 	 	      else if (r < 0.5)
	 	 	 	     imgQR[y*93+x] = 1; // dark
				else
					imgQR[y*93+x] = 2; // light
		}
	}
#endif
//--------

	fil = fopen(opt_output, "wb");
	if (fil == NULL) {
		fprintf(stderr, "Could not open output file\n");
		return -1;
	}

	im = gdImageCreateTrueColor(210, 210);
	for(int y=0; y<210; y++) {
		for (int x=0; x<210; x++) {
			int j = (y-12);
			int i = (x-12);

			if (i < 0 || j < 0 || i >= 186 || j>= 186) {
				int c = gdImageColorAllocate(im, 255, 255, 255);
				gdImageSetPixel(im, x, y, c);
			} else if (imgQR[(j>>1)*93+(i>>1)] == 1) {
				int c = gdImageColorAllocate(im, 0, 0, 0);
				gdImageSetPixel(im, x, y, c);
			} else if (imgQR[(j>>1)*93+(i>>1)] == 2) {
				int c = gdImageColorAllocate(im, 255, 255, 255);
				gdImageSetPixel(im, x, y, c);
			} else {
				int r = (unsigned char)(255*palette[quantized_image(i,j)].r);
				int g = (unsigned char)(255*palette[quantized_image(i,j)].g);
				int b = (unsigned char)(255*palette[quantized_image(i,j)].b);
				int c = gdImageColorAllocate(im, r, g, b);
				gdImageSetPixel(im, x, y, c);
			}
		}
	}
	gdImagePng(im, fil);
	gdImageDestroy(im);
	fclose(fil);
	logline("output written\n");

	return 0;
}
