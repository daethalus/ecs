#ifndef EVOLUTIO_POSITION_H
#define EVOLUTIO_POSITION_H

#include <algorithm>
#include <cmath>
#include <iostream>

using WorldIndex = long long;

struct Position {

    Position() {
        x = 0;
        y = 0;
        z = 0;
    }
    Position(float _x, float _y, int _z) {
        x = _x;
        y = _y;
        z = _z;
    }

	Position(int _x, int _y, int _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	Position(Position* pos) {
		x = pos->x;
		y = pos->y;
		z = pos->z;
	}

	Position(long value, int z) {
		x = (int)(value >> 32);
		y = (int)value;
		this->z = z;
	}

	Position* instance() {
		return new Position(x, y, z);
	}

	void update(Position pos) {
		this->x = pos.x;
		this->y = pos.y;
		this->z = pos.z;
	}

	WorldIndex worldIndex() {
		return  WorldIndex ((long) this->x) << 32 | ((long) this->y) & 0xffffffff;
	}

    float x;
    float y;
    short z;

    Position operator+(const Position& lhs){
        return Position(this->x + lhs.x, this->y + lhs.y, this->z + lhs.z);
    }

    Position operator-(const Position& lhs){
        return Position(this->x - lhs.x, this->y - lhs.y, this->z - lhs.z);
    }

    Position operator*(const Position& lhs){
        return Position(this->x * lhs.x, this->y * lhs.y, this->z * lhs.z);
    }

    Position operator+(const float& sum){
        return Position(this->x + sum, this->y + sum, this->z + sum);
    }

    bool operator() (const Position& lhs, const Position& rhs) const
    {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }

	bool operator == (const Position& lhs) const {
		return lhs.x == this->x && lhs.y == this->y && lhs.z == this->z;
	}
	bool operator > (const Position& lhs) const {
		return this->x > lhs.x || this->y > lhs.y  || this->z > lhs.z;
	}


	bool equalsFloor(Position lhs) {
		return floor(lhs.x) == floor(this->x) && floor(lhs.y) == floor(this->y) && floor(lhs.z) == floor(this->z);
	}

	bool equalsRound(Position lhs) {
		return round(lhs.x) == round(this->x) && round(lhs.y) == round(this->y) && round(lhs.z) == round(this->z);
	}

	friend std::ostream& operator<<(std::ostream& os, Position position) {
		os << "x: " << position.x << " y: " << position.y << " z: " << position.z;
		return os;
	}


	static Position lerp(Position a, Position b, float t) {
		t = std::clamp(t, 0.0f, 1.0f);
		return Position(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
	}

    bool operator<(const Position& lhs) const
    {
        if (x == lhs.x) {
            if (y == lhs.y) {
                return z < lhs.z;
            }
            return y < lhs.y ;
        }
        return x < lhs.x;
    }

    int toInt() {
        return (z * 16 * 16) + (y * 16) + x;
    }
};
#endif
