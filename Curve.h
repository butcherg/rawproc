#ifndef _curve_h
#define _curve_h

#define NDEBUG

#include "spline.h"
#include <vector>
#include <string>
#include <cmath>

typedef struct cp {
	double x, y;
}cp;

class Curve
{
	public:
		Curve() {
			loaded=false;
			controlpts.reserve(7);
			mn=-1.0;
			mx=-1.0;
		}

		std::vector<cp> getControlPoints()
		{
			return controlpts;
		}

		void setControlPoints(std::vector<cp> pts)
		{
			controlpts.clear();
			controlpts = pts;
			loaded=false;
		}

		void clearpoints()
		{
			controlpts.clear();
			loaded=false;
		}

		void insertpoint(double x, double y)
		{
			struct cp p;
			p.x = x;
			p.y = y;
			//if (mn>-1.0) if (p.x < mn) p.x = mn;
			//if (mx>-1.0) if (p.y > mx) p.y = mx;
			controlpts.push_back(p);
			loaded=false;
		}

		bool deletepoint(double x, double y)
		{
			for (unsigned int i=0; i<controlpts.size(); i++) {
				if (controlpts[i].x == x && controlpts[i].y == y) {
					controlpts.erase(controlpts.begin()+i);
					loaded=false;
					return true;
				}
			}
			loaded=false;
			return false;
		}

		void clampto(double min, double max)
		{
			mn = min;
			mx = max;
		}

		void scalepoints(double s)
		{
			for (unsigned int i=0; i<controlpts.size(); i++) {
				controlpts[i].x *= s;
				controlpts[i].y *= s;
			}
			loaded=false;
		}

		double getpoint(double x)
		{
			if (!loaded) setpoints();
			double y = s(x);
			if (mn > -1.0) if (y<mn) y=mn;
			if (mx > -1.0) if (y>mx) y=mx;
			return y;
		}

		cp getctrlpoint()
		{
			return ctrlpoint;
		}

		cp getctrlpoint(int i)
		{
			return controlpts[i];
		}

		void setctrlpoint(int i, cp p)
		{
			controlpts[i] = p;
			loaded=false;
		}


		//vector index of cp if found, -1 if not:
		int isctrlpoint(double x, double y, int radius)
		{
			double d=1000.0;
			double dp;
			int di = -1;
			for (unsigned int i=0; i<controlpts.size(); i++) {
				dp = sqrt(pow(controlpts[i].x-x,2.0)+pow(controlpts[i].y-y,2.0));
				if ((dp < d) & (dp < radius)) {
					d = dp;
					di = i;
				}
			}
			return di;
		}

		bool isendpoint(double x, double y, int radius)
		{
			int pt = isctrlpoint(x, y, radius);
			if (pt == 0) return true;
			if (pt == controlpts.size()-1) return true;
			return false;
		}

	private:


		bool loaded;
		std::vector<cp> controlpts;
		std::vector<double> X, Y;
		tk::spline s;
		cp ctrlpoint;
		double mn, mx;

		void sortpoints()
		{
			struct cp s;
			bool done;
			do {
				done = true;
				for (unsigned int i=0; i<controlpts.size()-1; i++) {
					if (controlpts[i].x > controlpts[i+1].x) {
						s = controlpts[i];
						controlpts[i] = controlpts[i+1];
						controlpts[i+1] = s;
						done = false;
					}
				}
			}
			while (!done);
		}

		void setpoints ()
		{
			X.clear(); Y.clear();
			sortpoints();
			for (unsigned int i=0; i<controlpts.size(); i++) {
				X.push_back(controlpts[i].x);
				Y.push_back(controlpts[i].y);
			}
			s.set_points(X,Y); 
			loaded = true;
		}

};


#endif
