#ifndef MATHSCONSTANTS_H
#define MATHSCONSTANTS_H

const double D_PI = 3.14159265358979323846;

const double D_2PI = 2.0*D_PI;

const double D2R   = D_PI/180.0;    ///<conv. factor from degrees to radians
const double R2D   = 180.0/D_PI;    ///<conv. factor from radians to degrees

template < typename Type >
inline const Type SQ(const Type& x) { return ((x)*(x)); }       ///<square

template < typename Type >
inline int SIGN(const Type& x) { return (((x)>0)?1:( ((x)==0)?0:-1)); } ///<sign

const float  F_PI  = (float)D_PI;
const double F_D2R = (float)D2R;

#endif // MATHSCONSTANTS_H
