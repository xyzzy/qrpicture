/*
  8.200(  0.370) Level:0 coarseW: 987 coarseH: 775 temperature:0.001000 repeat:0
  9.810(  1.610) pixels visited: 764925 changed:  19340
  9.920(  0.110) pixels visited:  68823 changed:   2208
  9.940(  0.020) pixels visited:  12075 changed:    392
  9.940(  0.000) pixels visited:   1692 changed:     34
  9.940(  0.000) pixels visited:    140 changed:      6
  9.940(  0.000) pixels visited:     30 changed:      1
  9.940(  0.000) pixels visited:      4 changed:      0
  9.950(  0.010) pixels visited:      1 changed:      0
 10.100(  0.150) pixels visited: 122632 changed:   1744
 10.150(  0.050) pixels visited:  34987 changed:    586
 10.160(  0.010) pixels visited:  10249 changed:    192
 10.160(  0.000) pixels visited:   3345 changed:     73
 10.160(  0.000) pixels visited:   1287 changed:     27
 10.160(  0.000) pixels visited:    466 changed:      9
 10.170(  0.010) pixels visited:    146 changed:      6
 10.170(  0.000) pixels visited:    107 changed:      5
 10.170(  0.000) pixels visited:     61 changed:      0
 10.460(  0.290) updated palette

  8.370(  0.380) Level:0 coarseW: 987 coarseH: 775 temperature:0.001000 repeat:0
 10.000(  1.630) pixels visited: 764925 changed:  19177
 10.240(  0.240) pixels visited: 169323 changed:   3410
 10.320(  0.080) pixels visited:  58396 changed:   1151
 10.340(  0.020) pixels visited:  19872 changed:    318
 10.350(  0.010) pixels visited:   5694 changed:     83
 10.350(  0.000) pixels visited:   1484 changed:     24
 10.350(  0.000) pixels visited:    455 changed:      8
 10.350(  0.000) pixels visited:    144 changed:      0
 10.660(  0.310) updated palette
*/

/* Copyright (c) 2006 Derrick Coetzee

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

int verbose = 0;

#define B_SCALE 256.0

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

		    a(i_x,i_y) *= B_SCALE;
	}
    }
}

void sum_coarsen(array2d< Pixel >& fine,
		 array2d< Pixel >& coarse)
{
    for(int y=0; y<coarse.get_height(); y++) {
	for(int x=0; x<coarse.get_width(); x++) {

	    double divisor = 1.0;
	    Pixel val = fine(x*2, y*2);
	    if (x*2 + 1 < fine.get_width())  {
		divisor += 1; val += fine(x*2 + 1, y*2);
	    }
	    if (y*2 + 1 < fine.get_height()) {
		divisor += 1; val += fine(x*2, y*2 + 1);
	    }
	    if (x*2 + 1 < fine.get_width() &&
		y*2 + 1 < fine.get_height()) {
		divisor += 1; val += fine(x*2 + 1, y*2 + 1);
	    }
	    coarse(x, y) = val ; // * (1.0/divisor);
	}
    }
}

int best_match_color(CoarseVariables &vars, int i_x, int i_y, int palette_size)
{
    int max_v = 0;
    double max_weight = vars(i_x, i_y, 0);
    for (int v=1; v < palette_size; v++) {
	if (vars(i_x, i_y, v) > max_weight) {
	    max_v = v;
	    max_weight = vars(i_x, i_y, v);
	}
    }
    return max_v;
}

void zoom_double(CoarseVariables& small, CoarseVariables& big, int coarse_level)
{
    const int small_width  = small.get_width() >> coarse_level;
    const int small_height = small.get_height() >> coarse_level;
    const int big_width  = small.get_width() >> (coarse_level-1);
    const int big_height = small.get_height() >> (coarse_level-1);

    for(int y=small_height-1; y>=0; y--) {
	for(int x=small_width-1; x>=0; x--) {
	    for(int z=0; z<small.get_depth(); z++) {
		big(2*x, 2*y, z) = small(x, y, z);
		big(2*x + 1, 2*y, z) = small(x, y, z); 
		big(2*x, 2*y + 1, z) = small(x, y, z);
		big(2*x + 1, 2*y + 1, z) = small(x, y, z);
	    }
	}
    }
    if ((big_width % 2) == 1) {
	int x = big_width - 1;
	for(int y=0; y<big_height; y++) {
	    for(int z=0; z<big.get_depth(); z++) {
		big(x,y,z) = big(x-1,y,z);
	    }
	}
    }
    if ((big_height % 2) == 1) {
	int y = big_height - 1;
	for(int x=0; x<big_width; x++) {
	    for(int z=0; z<big.get_depth(); z++) {
		big(x,y,z) = big(x,y-1,z);
	    }
	}
    }
    return;

#if 1
	/*!!!! THIS IS 0.2 CODE !!!!*/
    for(int y=0; y<small.get_height(); y++) {
	for(int x=0; x<small.get_width(); x++) {
	    for(int z=0; z<small.get_depth(); z++) {
		big(2*x, 2*y, z) = small(x, y, z);
		big(2*x + 1, 2*y, z) = small(x, y, z);
		big(2*x, 2*y + 1, z) = small(x, y, z);
		big(2*x + 1, 2*y + 1, z) = small(x, y, z);
	    }
	}
    }
    if ((big.get_width() % 2) == 1) {
	int x = big.get_width() - 1;
	for(int y=0; y<big.get_height(); y++) {
	    for(int z=0; z<big.get_depth(); z++) {
		big(x,y,z) = big(x-1,y,z);
	    }
	}
    }
    if ((big.get_height() % 2) == 1) {
	int y = big.get_height() - 1;
	for(int x=0; x<big.get_width(); x++) {
	    for(int z=0; z<big.get_depth(); z++) {
		big(x,y,z) = big(x,y-1,z);
	    }
	}
    }
#elif 1
	/*!!!! THIS IS 0.3 CODE !!!!*/
    for(int y=0; y<big.get_height()/2*2; y+=2) {
	for(int x=0; x<big.get_width()/2*2; x+=2) {
	    for(int z=0; z<big.get_depth(); z++) {
		int x_center = x/2, y_center = y/2;
		int x_left = x/2 - 1;
		if (x_left < 0) x_left++;
		int x_right = x/2 + 1;
		if (x_right >= small.get_width()) x_right--;
		int y_up = y/2 - 1;
		if (y_up < 0) y_up++;
		int y_down = y/2 + 1;
		if (y_down >= small.get_height()) y_down--;

		big(x, y, z) = 0.70*small(x_center, y_center, z) +
		               0.12*small(x_left,   y_center, z) +
		               0.12*small(x_center, y_up, z) +
	                       0.06*small(x_left,   y_up, z);
		big(x+1, y, z) = 0.70*small(x_center, y_center, z) +
		                 0.12*small(x_right,  y_center, z) +
		                 0.12*small(x_center, y_up, z) +
		                 0.06*small(x_right,  y_up, z);
		big(x, y+1, z) = 0.70*small(x_center, y_center, z) +
		                 0.12*small(x_left,   y_center, z) +
		                 0.12*small(x_center, y_down, z) +
		                 0.06*small(x_left,   y_down, z);
		big(x+1, y+1, z) = 0.70*small(x_center, y_center, z) +
		                   0.12*small(x_right,  y_center, z) +
		                   0.12*small(x_center, y_down, z) +
		                   0.06*small(x_right,  y_down, z);
	    }
	}
    }
    if ((big.get_width() % 2) == 1) {
	int x = big.get_width() - 1;
	for(int y=0; y<big.get_height(); y++) {
	    for(int z=0; z<big.get_depth(); z++) {
		big(x,y,z) = big(x-1,y,z);
	    }
	}
    }
    if ((big.get_height() % 2) == 1) {
	int y = big.get_height() - 1;
	for(int x=0; x<big.get_width(); x++) {
	    for(int z=0; z<big.get_depth(); z++) {
		big(x,y,z) = big(x,y-1,z);
	    }
	}
    }
#else
	/*!!!! THIS IS 0.4 CODE !!!!*/
    // Simple scaling of the weights array based on mixing the four
    // pixels falling under each fine pixel, weighted by area.
    // To mix the pixels a little, we assume each fine pixel
    // is 1.2 fine pixels wide and high.
    for(int y=0; y<big.get_height()/2*2; y++) {
	for(int x=0; x<big.get_width()/2*2; x++) {
	    double left = max(0.0, (x-0.1)/2.0), right  = min(small.get_width()-0.001, (x+1.1)/2.0);
	    double top  = max(0.0, (y-0.1)/2.0), bottom = min(small.get_height()-0.001, (y+1.1)/2.0);
	    int x_left = (int)floor(left), x_right  = (int)floor(right);
	    int y_top  = (int)floor(top),  y_bottom = (int)floor(bottom);
	    double area = (right-left)*(bottom-top);
	    double top_left_weight  = (ceil(left) - left)*(ceil(top) - top)/area;
	    double top_right_weight = (right - floor(right))*(ceil(top) - top)/area;
	    double bottom_left_weight  = (ceil(left) - left)*(bottom - floor(bottom))/area;
	    double bottom_right_weight = (right - floor(right))*(bottom - floor(bottom))/area;
	    double top_weight     = (right-left)*(ceil(top) - top)/area;
	    double bottom_weight  = (right-left)*(bottom - floor(bottom))/area;
	    double left_weight    = (bottom-top)*(ceil(left) - left)/area;
	    double right_weight   = (bottom-top)*(right - floor(right))/area;
	    for(int z=0; z<big.get_depth(); z++) {
		if (x_left == x_right && y_top == y_bottom) {
		    big(x, y, z) = small(x_left, y_top, z);
		} else if (x_left == x_right) {
		    big(x, y, z) = top_weight*small(x_left, y_top, z) +
			           bottom_weight*small(x_left, y_bottom, z);
		} else if (y_top == y_bottom) {
		    big(x, y, z) = left_weight*small(x_left, y_top, z) +
			           right_weight*small(x_right, y_top, z);
		} else {
		    big(x, y, z) = top_left_weight*small(x_left, y_top, z) +
			           top_right_weight*small(x_right, y_top, z) +
			           bottom_left_weight*small(x_left, y_bottom, z) +
			           bottom_right_weight*small(x_right, y_bottom, z);
		}
	    }
	}
    }
#endif
}

void refine_palette(array2d< double >& __s,
		    Pixel *__r,
		    Pixel *palette,
		    int palette_size)
{
    // We only computed the half of S above the diagonal - reflect it
    for (int v=0; v<palette_size; v++) {
	for (int alpha=0; alpha<v; alpha++) {
	    __s(v,alpha) = __s(alpha,v);
	}
    }

	// rewrite: vector<double> palette_channel = -1.0*((2.0*S_k).matrix_inverse())*R_k;

        double source[palette_size*palette_size];
        double result[palette_size*palette_size];

	if (1) {
		// Set result to identity matrix
		for(int i=0; i<palette_size; i++) {
			for(int j=0; j<palette_size; j++) {
				source[j*palette_size+i] = __s(i,j);
				result[j*palette_size+i] = 0;
			}
			result[i*palette_size+i] = 1;
		}

		// Reduce to echelon form, mirroring in result
		for(int i=0; i<palette_size; i++) {
			double mult = source[i*palette_size+i];
			for(int k=0; k<palette_size; k++) result[i*palette_size+k] = result[i*palette_size+k] / mult;
			for(int k=0; k<palette_size; k++) source[i*palette_size+k] = source[i*palette_size+k] / mult;
			for(int j=i+1; j<palette_size; j++) {
				double mult = source[j*palette_size+i];
				for(int k=0; k<palette_size; k++) result[j*palette_size+k] -= result[i*palette_size+k] * mult;
				for(int k=0; k<palette_size; k++) source[j*palette_size+k] -= source[i*palette_size+k] * mult;
			}
		}
		// Back substitute, mirroring in result
		for(int i=palette_size-1; i>=0; i--) {
			for(int j=i-1; j>=0; j--) {
				double mult = source[j*palette_size+i];
				for(int k=0; k<palette_size; k++) result[j*palette_size+k] -= result[i*palette_size+k] * mult;
				for(int k=0; k<palette_size; k++) source[j*palette_size+k] -= source[i*palette_size+k] * mult;
			}
		}
	}

	for(int row=0; row<palette_size; row++) {
		Pixel sum;
		for(int col=0; col<palette_size; col++)
			sum += __r[col] * result[row*palette_size+col];

		Pixel val = sum;
val.r /= 1;
val.g /= 1;
val.b /= 1;

if (val.r < 0 || val.g < 0 || val.b < 0) fprintf(stderr,"### %f %f %f\n", val.r, val.g, val.b);
if (val.r > 1 || val.g > 1 || val.b > 1) fprintf(stderr,"!!! %f %f %f\n", val.r, val.g, val.b);
		if (val.r < 0) val.r = 0; //!! NOT DETECTED
		if (val.r > 1) val.r = 1; //!! DETECTED
		if (val.g < 0) val.g = 0; //!! NOT DETECTED
		if (val.g > 1) val.g = 1; //!! DETECTED
		if (val.b < 0) val.b = 0; //!! NOT DETECTED
		if (val.b > 1) val.b = 1; //!! DETECTED
		palette[row] = val;
	}
}

void spatial_color_quant_n(
			 int coarse_level,
			 array2d< Pixel > fine_a,
			 CoarseVariables *p_coarse_variables,
                         Pixel *palette,
			 int palette_size,
			 int temps_per_level,

			 double initial_temperature,
			 double final_temperature,
			 double& temperature,
			 double& temperature_multiplier,

			 int fixed_palette,
			 int& rng_seed,
			 int image_width,
			 int image_height)
{
    // linear random generator with values of a,c,m as used in glibc
    int lcg = rng_seed;

    array2d< Pixel > __a(image_width >> coarse_level, image_height >> coarse_level); 
    sum_coarsen(fine_a, __a);

    const int level_width  = image_width  >> coarse_level;
    const int level_height = image_height >> coarse_level;
    const int level_size2d = level_width*level_height;

	int b00=0, b01=0, b11=0;
switch (coarse_level) {
case 1:
	b00 =  36; // 4*b0_00 + 4*b0_01 + b0_11;
	b01 = 120; // 4*b0_02 + 4*b0_01 + 2*b0_11 + 2*b0_12;
	b11 = 400; // 4*b0_11 + 8*b0_12 + 4*b0_22;
	break;
case 2:
	b00 =   36; // b1_00;
	b01 =  312; // 2*b1_00 + 2*b1_01;
	b11 = 2704; // 4*b1_00 + 8*b1_01 + 4*b1_11;
	break;
case 3:
	b00 =    36; // b2_00;
	b01 =   696; // 2*b2_00 + 2*b2_01;
	b11 = 13456; // 4*b2_00 + 8*b2_01 + 4*b2_11;
	break;
case 4:
	b00 =    36; // b3_00;
	b01 =  1464; // 2*b3_00 + 2*b3_01;
	b11 = 59536; // 4*b3_00 + 8*b3_01 + 4*b3_11;
	break;
default:
	b00 =     36; // b4_00;
	b01 =   3000; // 2*b4_00 + 2*b4_01;
	b11 = 250000; // 4*b4_00 + 8*b4_01 + 4*b4_11;
	break;
}
	const int ofs00 = &(*p_coarse_variables)(-1,-1,0) - &(*p_coarse_variables)(0,0,0);
	const int ofs01 = &(*p_coarse_variables)( 0,-1,0) - &(*p_coarse_variables)(0,0,0);
	const int ofs02 = &(*p_coarse_variables)(+1,-1,0) - &(*p_coarse_variables)(0,0,0);
	const int ofs10 = &(*p_coarse_variables)(-1, 0,0) - &(*p_coarse_variables)(0,0,0);
	const int ofs11 = &(*p_coarse_variables)( 0, 0,0) - &(*p_coarse_variables)(0,0,0);
	const int ofs12 = &(*p_coarse_variables)(+1, 0,0) - &(*p_coarse_variables)(0,0,0);
	const int ofs20 = &(*p_coarse_variables)(-1,+1,0) - &(*p_coarse_variables)(0,0,0);
	const int ofs21 = &(*p_coarse_variables)( 0,+1,0) - &(*p_coarse_variables)(0,0,0);
	const int ofs22 = &(*p_coarse_variables)(+1,+1,0) - &(*p_coarse_variables)(0,0,0);

if (level_size2d <= 400 || coarse_level == 4) {

	temperature_multiplier += temps_per_level;
fprintf(stderr,"%f\n", temperature_multiplier);
	temperature_multiplier = pow(final_temperature/initial_temperature, 1.0/(temperature_multiplier-1));
fprintf(stderr,"%f %f %f\n", temperature_multiplier, initial_temperature, final_temperature);

    for(int j=0; j<level_height; j++) {
        for(int i=0; i<level_width; i++) {
            for(int k=0; k<p_coarse_variables->get_depth(); k++) {
		lcg = lcg * 1103515245 + 12345;
                (*p_coarse_variables)(i,j,k) = (lcg&0x7fffffff) / (double)0x7fffffff;
	    }
	}
    }
    p_coarse_variables->clear_border();

} else {
	temperature_multiplier += temps_per_level;
	spatial_color_quant_n(coarse_level+1, 
				__a, p_coarse_variables, 
				palette, palette_size, 
				temps_per_level, initial_temperature,final_temperature,temperature, temperature_multiplier, 
				fixed_palette, rng_seed, image_width, image_height);
}

    array2d< double > __s(palette_size, palette_size);
    for (int iters_at_current_level=0; iters_at_current_level<temps_per_level; iters_at_current_level++) {


	    logline("Level:%d coarseW:%4d coarseH:%4d temperature:%f\n", coarse_level, level_width,  level_height, temperature);

	    int pixels_changed = 0, pixels_visited = 0;

		char *dirty_flag = new char[level_size2d];
		int *dirty_xy = new int[level_size2d];
		int dirty_in = 0, dirty_out = 0, dirty_cnt = 0, dirty_step = 0;

		// visit all pixels
		for (dirty_in=0; dirty_in<level_size2d; dirty_in++) {
			dirty_flag[dirty_in] = 1;
			dirty_xy[dirty_in] = dirty_in;
		}
		dirty_cnt = level_size2d;
		dirty_in %= level_size2d;
		dirty_step = dirty_cnt;

		// randomize
		for (int i=1; i<level_size2d; i++) {
			lcg = lcg * 1103515245 + 12345;
			int j =  (lcg&0x7fffffff) % (i + 1);

			int swap = dirty_xy[i];
			dirty_xy[i] = dirty_xy[j];
			dirty_xy[j] = swap;
		}

	    // Compute 2*sum(j in extended neighborhood of i, j != i) b_ij

const double const1 = (-2 / B_SCALE / temperature);
	    while(dirty_cnt)
	    {
			const int i_x = dirty_xy[dirty_out]%level_width;
			const int i_y = dirty_xy[dirty_out]/level_width;
			dirty_out = (dirty_out+1)%level_size2d;
			dirty_cnt--;


		// Compute (25)
		Pixel p_i;

			for (int alpha=0; alpha < palette_size; alpha++) {

				double *pdata = &(*p_coarse_variables)(i_x,i_y,alpha);

				double sum = 0;
				sum += (pdata[ofs00] + pdata[ofs02] +  pdata[ofs20] +  pdata[ofs22]) * b00;
				sum += (pdata[ofs01] + pdata[ofs10] +  pdata[ofs12] +  pdata[ofs21]) * b01;

				p_i += palette[alpha] * sum;
			}

		double meanfield_logs[palette_size];
		double max_meanfield_log;
		double meanfield_sum = 0.0;

		    double tmpR = p_i.r - __a(i_x, i_y).r + palette[0].r*b11/2;
		    double tmpG = p_i.g - __a(i_x, i_y).g + palette[0].g*b11/2;
		    double tmpB = p_i.b - __a(i_x, i_y).b + palette[0].b*b11/2;
		    meanfield_logs[0] = (palette[0].r*tmpR + palette[0].g*tmpG + palette[0].b*tmpB) * const1;
		    max_meanfield_log = meanfield_logs[0];

		for (int v=0; v < palette_size; v++) {

		    double tmpR = p_i.r - __a(i_x, i_y).r + palette[v].r*b11/2;
		    double tmpG = p_i.g - __a(i_x, i_y).g + palette[v].g*b11/2;
		    double tmpB = p_i.b - __a(i_x, i_y).b + palette[v].b*b11/2;
		    meanfield_logs[v] = (palette[v].r*tmpR + palette[v].g*tmpG + palette[v].b*tmpB) * const1;

		    if (meanfield_logs[v] > max_meanfield_log)
			max_meanfield_log = meanfield_logs[v];
		}

		double meanfields[palette_size];
		for (int v=0; v < palette_size; v++) {
		    meanfields[v] = exp(meanfield_logs[v]-max_meanfield_log+100);
		    meanfield_sum += meanfields[v];
		}
		if (meanfield_sum == 0) {
		    fprintf(stderr, "Fatal error: Meanfield sum underflowed. Please contact developer.\n");
		    exit(-1);
		}

		for (int v=0; v < palette_size; v++) {
		    double new_val = meanfields[v]/meanfield_sum;
		    // Prevent the matrix S from becoming singular
		    if (new_val <= 0) new_val = 1e-10;
		    if (new_val >= 1) new_val = 1 - 1e-10;
		    (*p_coarse_variables)(i_x,i_y,v) = new_val;
		    pixels_changed++;
		}

		pixels_visited++;
		if (--dirty_step <= 0) {
			logline("pixels visited:%7d changed:%7d\n", pixels_visited, pixels_changed);
			dirty_step = dirty_cnt;
			pixels_visited = pixels_changed = 0;
		}
	    }

		delete[] dirty_flag;
		delete[] dirty_xy;

		if (!fixed_palette) {
			// compute initial s
			for (int alpha=0; alpha<palette_size; alpha++) for (int v=0; v<=alpha; v++) __s(v,alpha) = 0;
			for (int i_y=0; i_y<level_height; i_y++) {
			for (int i_x=0; i_x<level_width; i_x++) {
			for (int a=0; a<palette_size; a++) {

				double *pdata = &(*p_coarse_variables)(i_x,i_y,a);

				double sum = 0;
				sum += (pdata[ofs00] + pdata[ofs02] +  pdata[ofs20] +  pdata[ofs22]) * b00;
				sum += (pdata[ofs01] + pdata[ofs10] +  pdata[ofs12] +  pdata[ofs21]) * b01;

				for (int v=0; v<=a; v++)
					__s(v,a) += (*p_coarse_variables)(i_x,i_y,v) * sum;

				__s(a,a) += pdata[ofs11] * b11;
			}}}

			Pixel __r[palette_size];
			for (int i_y=0; i_y<level_height; i_y++) {
			for (int i_x=0; i_x<level_width; i_x++) {
			for (int a=0; a<palette_size; a++) {
				__r[a] += __a(i_x,i_y) * (*p_coarse_variables)(i_x,i_y,a);
			}}}

			// refine palette using s
			refine_palette(__s, __r, palette, palette_size);

			logline("updated palette\n");
		}

        temperature *= temperature_multiplier;
    }
    zoom_double(*p_coarse_variables, *p_coarse_variables, coarse_level);
}


void spatial_color_quant(array2d< Pixel >& image,
                         array2d< int >& quantized_image,
                         Pixel *palette,
			 int palette_size,
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

const int b0_00 =  1; //   filter00*filter00;
const int b0_01 =  4; // 2*filter00*filter01;
const int b0_02 =  6; // 2*filter00*filter00 + filter01*filter01;
const int b0_11 = 16; // 2*filter00*filter11 + 2*filter01*filter01;
const int b0_12 = 24; // 4*filter00*filter01 + 2*filter01*filter11;
const int b0_22 = 36; // 4*filter00*filter00 + 4*filter01*filter01 + filter11*filter11;

    int coarse_level = 0;
    int image_width = image.get_width();
    int image_height = image.get_height();

    // Compute a_i, b_{ij} according to (11)

    // linear random generator with values of a,c,m as used in glibc
    int lcg = rng_seed;

    array2d< Pixel > a0(image_width, image_height);
    compute_a_image(image, b0_00/256.0,b0_01/256.0,b0_02/256.0,b0_11/256.0,b0_12/256.0,b0_22/256.0, a0);

    CoarseVariables *p_coarse_variables = new CoarseVariables(image_width  >> coarse_level, image_height >> coarse_level, palette_size);

//#if 0
//	/*!!! THIS IS 0.4 !!!*/
//    double temperature_multiplier = pow(final_temperature/initial_temperature, 1.0/(max(3, max_coarse_level*temps_per_level)));
//#else
//	/*!!! THIS IS 0.2 !!!*/
//    double temperature_multiplier = pow(final_temperature/initial_temperature, 1.0/(max_coarse_level*temps_per_level));
//#endif

    double temperature = initial_temperature;
    double temperature_multiplier = 1+temps_per_level; // temps_per_level*repeats_per_temp;
    spatial_color_quant_n(coarse_level+1, 
				a0, p_coarse_variables, 
				palette, palette_size, 
				temps_per_level, initial_temperature,final_temperature,temperature, temperature_multiplier, 
				fixed_palette, rng_seed, image_width, image_height);

	const int ofs00 = &(*p_coarse_variables)(-2,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs01 = &(*p_coarse_variables)(-1,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs02 = &(*p_coarse_variables)( 0,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs03 = &(*p_coarse_variables)(+1,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs04 = &(*p_coarse_variables)(+2,-2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs10 = &(*p_coarse_variables)(-2,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs11 = &(*p_coarse_variables)(-1,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs12 = &(*p_coarse_variables)( 0,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs13 = &(*p_coarse_variables)(+1,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs14 = &(*p_coarse_variables)(+2,-1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs20 = &(*p_coarse_variables)(-2, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs21 = &(*p_coarse_variables)(-1, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs22 = &(*p_coarse_variables)( 0, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs23 = &(*p_coarse_variables)(+1, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs24 = &(*p_coarse_variables)(+2, 0,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs30 = &(*p_coarse_variables)(-2,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs31 = &(*p_coarse_variables)(-1,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs32 = &(*p_coarse_variables)( 0,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs33 = &(*p_coarse_variables)(+1,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs34 = &(*p_coarse_variables)(+2,+1,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs40 = &(*p_coarse_variables)(-2,+2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs41 = &(*p_coarse_variables)(-1,+2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs42 = &(*p_coarse_variables)( 0,+2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs43 = &(*p_coarse_variables)(+1,+2,0) - &(*p_coarse_variables)(0,0,0) ;
	const int ofs44 = &(*p_coarse_variables)(+2,+2,0) - &(*p_coarse_variables)(0,0,0) ;

    array2d< double > __s(palette_size, palette_size);
    int iters_at_current_level=0;
    while (temperature > final_temperature || iters_at_current_level++<temps_per_level) {

	int coarse_width = p_coarse_variables->get_width();
	int coarse_height = p_coarse_variables->get_height();
	int coarse_size2d = coarse_width*coarse_height;

	for(int repeat=0; repeat<repeats_per_temp; repeat++)
	{
	    logline("Level:%d coarseW:%4d coarseH:%4d temperature:%f repeat:%d iter:%d\n", coarse_level, coarse_width,  coarse_height, temperature, repeat, iters_at_current_level);

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

const double const1 = (-2 / B_SCALE / temperature);

	    while(dirty_cnt)
	    {
			const int i_x = dirty_xy[dirty_out]%coarse_width;
			const int i_y = dirty_xy[dirty_out]/coarse_width;
			dirty_out = (dirty_out+1)%coarse_size2d;
			dirty_cnt--;


		// Compute (25)
		Pixel p_i;

			for (int alpha=0; alpha < palette_size; alpha++) {

				double *pdata = &(*p_coarse_variables)(i_x,i_y,alpha);

				double sum = 0;
				sum += (pdata[ofs00] + pdata[ofs04] +  pdata[ofs40] +  pdata[ofs44]) * b0_00;
				sum += (pdata[ofs01] + pdata[ofs10] +  pdata[ofs03] +  pdata[ofs30]+
					pdata[ofs14] + pdata[ofs41] +  pdata[ofs34] +  pdata[ofs43]) * b0_01;
				sum += (pdata[ofs02] + pdata[ofs20] +  pdata[ofs24] +  pdata[ofs42]) * b0_02;
				sum += (pdata[ofs11] + pdata[ofs13] +  pdata[ofs31] +  pdata[ofs33]) * b0_11;
				sum += (pdata[ofs12] + pdata[ofs21] +  pdata[ofs32] +  pdata[ofs23]) * b0_12;

				p_i += palette[alpha] * sum;
			}

		double meanfield_logs[palette_size];
		double max_meanfield_log = 0;
		double meanfield_sum = 0.0;

		    meanfield_logs[0]= ( (palette[0].r*b0_22/2 + p_i.r - a0(i_x, i_y).r)*palette[0].r +
					 (palette[0].g*b0_22/2 + p_i.g - a0(i_x, i_y).g)*palette[0].g +
					 (palette[0].b*b0_22/2 + p_i.b - a0(i_x, i_y).b)*palette[0].b ) * const1;
		    max_meanfield_log = meanfield_logs[0];

		for (int v=1; v < palette_size; v++) {

		    meanfield_logs[v]= ( (palette[v].r*b0_22/2 + p_i.r - a0(i_x, i_y).r)*palette[v].r +
					 (palette[v].g*b0_22/2 + p_i.g - a0(i_x, i_y).g)*palette[v].g +
					 (palette[v].b*b0_22/2 + p_i.b - a0(i_x, i_y).b)*palette[v].b ) * const1;

		    if (meanfield_logs[v] > max_meanfield_log)
			max_meanfield_log = meanfield_logs[v];
		}
		double meanfields[palette_size];
		for (int v=0; v < palette_size; v++) {
		    meanfields[v] = exp(meanfield_logs[v]-max_meanfield_log+100);
		    if (meanfields[v] < 0) meanfields[v] = 0;
		    meanfield_sum += meanfields[v];
		}
		if (meanfield_sum == 0) {
		    fprintf(stderr, "Fatal error: Meanfield sum underflowed. Please contact developer.\n");
		    exit(-1);
		}

		int old_max_v = -1;
		double old_max_weight = -1;
		int new_max_v = -1;
		double new_max_weight = -1;

		for (int v=0; v < palette_size; v++) {
		    double new_val = meanfields[v]/meanfield_sum;
		    double old_val = (*p_coarse_variables)(i_x,i_y,v);

		    // update best match
		    if (old_val > old_max_weight) {
			old_max_v = v;
			old_max_weight = old_val;
		    }

		    (*p_coarse_variables)(i_x,i_y,v) = new_val;

		    // update best match
		    if (new_val > new_max_weight) {
			new_max_v = v;
			new_max_weight = new_val;
		    }
		}
		dirty_flag[i_y*coarse_width+i_x] = 0;

		if (old_max_v != new_max_v) {
		    pixels_changed++;
		    // We don't add the outer layer of pixels , because
		    // there isn't much weight there, and if it does need
		    // to be visited, it'll probably be added when we visit
		    // neighboring pixels.
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

		pixels_visited++;
		if (--dirty_step <= 0) {
			logline("pixels visited:%7d changed:%7d\n", pixels_visited, pixels_changed);
			dirty_step = dirty_cnt;
			pixels_visited = pixels_changed = 0;
		}
	    }

		delete[] dirty_flag;
		delete[] dirty_xy;

		if (!fixed_palette) {
			Pixel __r[palette_size];

			// compute initial s
			for (int alpha=0; alpha<palette_size; alpha++) for (int v=0; v<=alpha; v++) __s(v,alpha) = 0;
			for (int i_y=0; i_y<coarse_height; i_y++) {
			for (int i_x=0; i_x<coarse_width; i_x++) {
			for (int a=0; a<palette_size; a++) {

				double *pdata = &(*p_coarse_variables)(i_x,i_y,a);

				double sum = 0;
				sum += (pdata[ofs00] + pdata[ofs04] +  pdata[ofs40] +  pdata[ofs44]) * b0_00;
				sum += (pdata[ofs01] + pdata[ofs10] +  pdata[ofs03] +  pdata[ofs30]+
					pdata[ofs14] + pdata[ofs41] +  pdata[ofs34] +  pdata[ofs43]) * b0_01;
				sum += (pdata[ofs02] + pdata[ofs20] +  pdata[ofs24] +  pdata[ofs42]) * b0_02;
				sum += (pdata[ofs11] + pdata[ofs13] +  pdata[ofs31] +  pdata[ofs33]) * b0_11;
				sum += (pdata[ofs12] + pdata[ofs21] +  pdata[ofs32] +  pdata[ofs23]) * b0_12;

				for (int v=0; v<=a; v++)
					__s(v,a) += (*p_coarse_variables)(i_x,i_y,v) * sum;

				__s(a,a) += pdata[ofs22] * b0_22;

				__r[a] += a0(i_x,i_y) * pdata[ofs22];
			}}}

			// refine palette using s
			refine_palette(__s, __r, palette, palette_size);

			logline("updated palette\n");
		}

		if (verbose > 1) {
			char fname[128];
			static int seqnr = 0;
			sprintf(fname, "tmp-%02d-%d-%d-%d.png", seqnr++, coarse_level, coarse_width, coarse_height);

			FILE *fil = fopen(fname, "wb");
			if (fil) {
				gdImagePtr im = gdImageCreateTrueColor(coarse_width, coarse_height);

				for(int y=0; y<coarse_height; y++) {
					for (int x=0; x<coarse_width; x++) {
						int v = best_match_color(*p_coarse_variables, x, y, palette_size);
						int r = (unsigned char)(255*palette[v].r);
						int g = (unsigned char)(255*palette[v].g);
						int b = (unsigned char)(255*palette[v].b);
						int c = gdImageColorAllocate(im, r, g, b);
						gdImageSetPixel(im, x, y, c);
					}
				}
				gdImagePng(im, fil);
				gdImageDestroy(im);
			}
			fclose(fil);
		}
	}

	temperature *= temperature_multiplier;
	if (temperature < final_temperature)
		temperature = final_temperature;
    }

	int palette_count[palette_size];
	for (int i=0; i<palette_size; i++)
		palette_count[i] = 0;

    for(int i_x = 0; i_x < image_width; i_x++) {
	for(int i_y = 0; i_y < image_height; i_y++) {
	    int c = best_match_color(*p_coarse_variables, i_x, i_y, palette_size);
	    palette_count[c]++;
	    quantized_image(i_x,i_y) = c;
	}
    }

    if (verbose > 1) {
	for (int v=0; v<palette_size; v++)
		printf("%f %f %f %9d\n", palette[v].r, palette[v].g, palette[v].b, palette_count[v]);
    }
}

void usage(const char *argv0, int verbose)
{
	printf("Usage: %s [options] <source image> <desired palette size> <output image>\n", argv0);
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
	--dither=n	dithering\n\
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

int main(int argc, char* argv[]) {

double opt_initial_temperature = 1.000;
double opt_final_temperature = 0.001;
int opt_tpl = 3;
int opt_rpt = 1;
int opt_verbose = 0;
int opt_width = 0;
int opt_height = 0;
int opt_seed = 0;
int opt_filter_size = 3;
double opt_dither = 0;
char *opt_source;
int opt_palette_size;
char *opt_output;
const char *opt_palette = NULL;

gdImagePtr im = NULL;
FILE *fil;

	for (;;) {
		int option_index = 0;
		enum {	LO_HELP=1, LO_INITIAL, LO_FINAL, LO_TPL, LO_RPT, LO_PALETTE,
			LO_VERBOSE='v', LO_W='w', LO_H='h', LO_SEED='s', LO_FILTER='f', LO_DITHER='d' };
		static struct option long_options[] = {
			/* name, has_arg, flag, val */
			{"help", 0, 0, LO_HELP},
			{"verbose", 0, 0, LO_VERBOSE},
			{"width", 1, 0, LO_W},
			{"height", 1, 0, LO_H},
			{"seed", 1, 0, LO_SEED},
			{"filter-size", 1, 0, LO_FILTER},
			{"dither", 1, 0, LO_DITHER},
			{"initial-temperature", 1, 0, LO_INITIAL},
			{"final-temperature", 1, 0, LO_FINAL},
			{"temperature-per-level", 1, 0, LO_TPL},
			{"repeat-per-temperature", 1, 0, LO_RPT},
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
			opt_filter_size = strtol(optarg, NULL, 10);
			if (opt_filter_size != 1 && opt_filter_size != 3 && opt_filter_size != 5) {
				fprintf(stderr, "Filter size must be one of 1, 3, or 5.\n");
				return -1;
			}
			break;
		case LO_DITHER:
			opt_dither = strtod(optarg, NULL);
			if (opt_dither <= 0) {
				printf("Dithering level must be more than zero.\n");
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

	if (argc-optind < 3) {
		usage(argv[0], 1);
		exit(1);
	}
	opt_source = argv[optind++];
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

	/* open source */
	fil = fopen(opt_source, "rb");
	if (fil == NULL) {
		fprintf(stderr, "Could not open source image\n");
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

	const int width = opt_width ? opt_width : gdImageSX(im);
	const int height = opt_height ? opt_height : gdImageSY(im);
	if (width <= 0 || height <= 0) {
		fprintf(stderr, "Must specify a valid positive image width and height with --width and --height.\n");
		return -1;
	}

	Pixel palette[1024];
	if (opt_palette) {
		opt_palette_size = 0;
		FILE* in = fopen(opt_palette, "r");
		if (in == NULL) {
			fprintf(stderr, "Could not open palette file\n");
			return -1;
		}
		double r,g,b;
		while (fscanf(in, "%lf %lf %lf\n", &r,  &g, &b) == 3) {
			if (opt_palette_size+1 < 1024)
				palette[opt_palette_size++] = Pixel(r,g,b);
		}
		fclose(in);
	} else {
		for (int i=0; i<opt_palette_size; i++) {
			Pixel p;
			p.r = ((double)rand())/RAND_MAX;
			p.g = ((double)rand())/RAND_MAX;
			p.b = ((double)rand())/RAND_MAX;
			palette[i] = p;
		}
	}

	fprintf(stderr,"invocation: \"%s\" %d \"%s\" --width=%d --height=%d --seed=%d --filter-size=%d --dither=%f --initial-temperature=%f --final-temperature=%f --temperature-per-level=%d --repeat-per-temperature=%d --palette=\"%s\"\n",
		opt_source,opt_palette_size,opt_output,width,height,opt_seed,opt_filter_size,opt_dither,
		opt_initial_temperature,opt_final_temperature,opt_tpl,opt_rpt,
		opt_palette?opt_palette:"");

	array2d< Pixel > image(width, height);
	array2d< int > quantized_image(width, height);

	/* load source */
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int v = gdImageGetTrueColorPixel(im, x, y);
			image(x,y).r = ((v>>16) & 0xFF) / 255.0L;
			image(x,y).g = ((v>>8) & 0xFF) / 255.0L;
			image(x,y).b = (v & 0xFF) / 255.0L;
		}
	}
	gdImageDestroy(im);
	logline("source read\n");

	double stddev;
	if (opt_dither)
		stddev = opt_dither;
	else
		stddev = 0.09*log((double)image.get_width()*image.get_height()) - 0.04*log((double)opt_palette_size) + 0.001;

	spatial_color_quant(image, quantized_image, palette, opt_palette_size, opt_initial_temperature, opt_final_temperature,opt_tpl,opt_rpt, opt_palette!=NULL, opt_seed);

	logline("converted\n");

	fil = fopen(opt_output, "wb");
	if (fil == NULL) {
		fprintf(stderr, "Could not open output file\n");
		return -1;
	}

	im = gdImageCreateTrueColor(width, height);

	for(int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			int r = (unsigned char)(255*palette[quantized_image(x,y)].r);
			int g = (unsigned char)(255*palette[quantized_image(x,y)].g);
			int b = (unsigned char)(255*palette[quantized_image(x,y)].b);
			int c = gdImageColorAllocate(im, r, g, b);
			gdImageSetPixel(im, x, y, c);
		}
	}
	gdImagePng(im, fil);
	gdImageDestroy(im);
	fclose(fil);
	logline("output written\n");

	return 0;
}
