#ifndef CHARACTER_CONTROLLER_H
#define CHARACTER_CONTROLLER_H

#include "window.h"

class characterController {

public:

	float _x,_y,_z;

	float _v;

	characterController(float x, float y, float z) : _x(x), _y(y), _z(z), _v(0)
	{}
	
	void update(const float dt) {

		


	}

};


#endif // CHARACTER_CONTROLLER_H
