/* 	A sensor that returns a positive value when the object in which
	it is embedded comes in contact with an external object,
	and is negative otherwise. */

#ifndef _TOUCH_SENSOR_H
#define _TOUCH_SENSOR_H

class TOUCH_SENSOR {

public:
	class OBJECT *containerObject;

private:
	double value;

public:
	TOUCH_SENSOR(OBJECT *cO);
	~TOUCH_SENSOR(void);
	void	Activate(void);
	void	Clear(void);
	double	Difference(TOUCH_SENSOR *other);
	double	Get_Value(void);
	void	Print(void);
	void	Reset(void);
};

#endif
