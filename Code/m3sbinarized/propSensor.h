/* A sensor that returns a value commensurate with the
flexion or extension of the joint to which it is attached. */

#ifndef _PROP_SENSOR_H
#define _PROP_SENSOR_H

class PROP_SENSOR {

private:
	double value;

public:
	PROP_SENSOR(void);
	~PROP_SENSOR(void);
	double Difference(PROP_SENSOR *other);
	double 	Get_Value(void);
	void    Reset(void);
	void 	Update(double v);
	void	Write(void);
};

#endif
