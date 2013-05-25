#include "exifrational.h"

ExifURational::ExifURational():
	QPair<first_type, second_type>(0, 1)
{
}

ExifURational::ExifURational(first_type f, second_type s):
	QPair<first_type, second_type>(f, s)
{
}

qreal ExifURational::toReal() const
{
	return (qreal)first / second;
}

ExifSRational::ExifSRational():
	QPair<first_type, second_type>(0, 1)
{
}

ExifSRational::ExifSRational(first_type f, second_type s):
	QPair<first_type, second_type>(f, s)
{
}

qreal ExifSRational::toReal() const
{
	return (qreal)first / second;
}
