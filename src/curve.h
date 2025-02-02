#ifndef _curve_h
#define _curve_h

#ifndef NDEBUG
#define NDEBUG
#endif

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>

#include "spline.h"

typedef struct cp {
	double x, y;
}cp;

/*

// spline interpolation
class spline {
	private:
		std::vector<double> m_x,m_y;           // x,y coordinates of points
		// interpolation parameters
		// f(x) = a*(x-x_i)^3 + b*(x-x_i)^2 + c*(x-x_i) + y_i
		std::vector<double> m_a,m_b,m_c,m_d;
	public:
		void set_points(const std::vector<double>& x,
			const std::vector<double>& y, bool cubic_spline=true);
		double operator() (double x) const;
};

*/

class Curve
{
	public:
		Curve();
		Curve(double lx, double ly, double hx, double hy);
		std::vector<cp> getControlPoints();
		void setControlPoints(std::vector<cp> pts);
		void clearpoints();
		void insertpoint(double x, double y);
		bool deletepoint(double x, double y);
		void clampto(double min, double max);
		void scalepoints(double s);
		double getpoint(double x);
		cp getctrlpoint();
		cp getctrlpoint(int i);
		void setctrlpoint(int i, cp p);
		//vector index of cp if found, -1 if not:
		int isctrlpoint(double x, double y, int radius);
		bool isendpoint(double x, double y, int radius);

	private:

		std::vector<cp> controlpts;
		std::vector<double> X, Y;
		tk::spline s;
		cp ctrlpoint;
		double mn, mx, slope;

		void sortpoints();
		void setpoints ();

};


/*
 * spline.h
 *
 * simple cubic spline interpolation library without external
 * dependencies
 *
 * ---------------------------------------------------------------------
 * Copyright (C) 2011, 2014 Tino Kluge (ttk448 at gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ---------------------------------------------------------------------
 *
 */
 
 /*

class band_matrix {
private:
   std::vector< std::vector<double> > m_upper;  // upper band
   std::vector< std::vector<double> > m_lower;  // lower band
public:
   band_matrix() {};                             // constructor
   band_matrix(int dim, int n_u, int n_l);       // constructor
   ~band_matrix() {};                            // destructor
   void resize(int dim, int n_u, int n_l);      // init with dim,n_u,n_l
   int dim() const;                             // matrix dimension
   int num_upper() const {
      return m_upper.size()-1;
   }
   int num_lower() const {
      return m_lower.size()-1;
   }
   // access operator
   double & operator () (int i, int j);            // write
   double   operator () (int i, int j) const;      // read
   // we can store an additional diogonal (in m_lower)
   double& saved_diag(int i);
   double  saved_diag(int i) const;
   void lu_decompose();
   std::vector<double> r_solve(const std::vector<double>& b) const;
   std::vector<double> l_solve(const std::vector<double>& b) const;
   std::vector<double> lu_solve(const std::vector<double>& b,
                                bool is_lu_decomposed=false);

};

*/



#endif
