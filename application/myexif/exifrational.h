#ifndef EXIFRATIONAL_H
#define EXIFRATIONAL_H

#include <QPair>

class ExifURational : public QPair<quint32, quint32>
{
public:
	ExifURational();
	ExifURational(first_type f, second_type s);
	qreal toReal() const;
};

class ExifSRational : public QPair<qint32, qint32>
{
public:
	ExifSRational();
	ExifSRational(first_type f, second_type s);
	qreal toReal() const;
};


#endif // EXIFRATIONAL_H
