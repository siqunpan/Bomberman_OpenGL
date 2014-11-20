#include "MyMath.h"

MyMath::MyMath()
{
	
}

point MyMath::produit_vertoriel(point point1, point point2)
{
	point result;

	result.x = point1.y*point2.z - point1.z*point2.y;
	result.y = point1.z*point2.x - point1.x*point2.z;
	result.z = point1.x*point2.y - point1.y*point2.x;

	return result;
}