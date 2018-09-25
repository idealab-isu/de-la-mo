#include "LayerSurface.h"

LayerSurface::LayerSurface()
{
	this->init_vars();
}

LayerSurface::LayerSurface(const LayerSurface& rhs)
{
	this->init_vars();
	this->copy_vars(rhs, *this);
}

LayerSurface::~LayerSurface()
{
	this->delete_vars();
}

LayerSurface& LayerSurface::operator=(const LayerSurface& rhs)
{
	if (this != &rhs)
	{
		this->copy_vars(rhs, *this);
	}
	return *this;
}

void LayerSurface::copy(LayerSurface& lhs)
{
	this->copy_vars(*this, lhs);
}

void LayerSurface::init_vars()
{
	this->_mId = 0;
	this->_mAngle = 0;
	this->_eDelaminationType = DelaminationType::COHESIVE;
	this->_bInitialSurface = false;
	this->_pOwner = nullptr;
	this->_pSurfPair = nullptr;
	// Objects which belong to 3rd party solid modeling libraries are always set to NULL
	this->_mFace = NULL;
	this->_bStiffenerGenerated = false;
	this->_bStiffenerPaired = false;
	this->_pCreatedFrom = nullptr;
	this->_eSurfDir = Direction::NODIR;
}

void LayerSurface::delete_vars()
{
	this->_pOwner = nullptr;
	this->_pSurfPair = nullptr;
	// Objects which belong to 3rd party solid modeling libraries are always set to NULL
	this->_mFace = NULL;
	this->_pCreatedFrom = nullptr;
}

void LayerSurface::copy_vars(const LayerSurface& rhs, LayerSurface& lhs)
{
	lhs._mId = rhs._mId;
	lhs._mFace = rhs._mFace;
	lhs._mPoint = rhs._mPoint;
	lhs._mNormal = rhs._mNormal;
	lhs._mAngle = rhs._mAngle;
	lhs._bInitialSurface = rhs._bInitialSurface;
	lhs._eSurfDir = rhs._eSurfDir;
	lhs._pOwner = rhs._pOwner;
	lhs._eDelaminationType = rhs._eDelaminationType;
	lhs._pSurfPair = rhs._pSurfPair;
	lhs._bStiffenerGenerated = rhs._bStiffenerGenerated;
	lhs._bStiffenerPaired = rhs._bStiffenerPaired;
	lhs._pCreatedFrom = rhs._pCreatedFrom;
}

DelaminationType LayerSurface::delam_type()
{
	return this->_eDelaminationType;
}

void LayerSurface::delam_type(DelaminationType status)
{
	this->_eDelaminationType = status;
}

delamo::TPoint3<double> LayerSurface::point_coords()
{
	delamo::TPoint3<double> ret(this->_mPoint.x(), this->_mPoint.y(), this->_mPoint.z());
	return ret;
}

delamo::TPoint3<double> LayerSurface::normal_coords()
{
	delamo::TPoint3<double> ret(this->_mNormal.x(), this->_mNormal.y(), this->_mNormal.z());
	return ret;
}

DLM_FACEP LayerSurface::face()
{
	return this->_mFace;
}

void LayerSurface::face(DLM_FACEP face)
{
	this->_mFace = face;
}

DLM_POSITION LayerSurface::point()
{
	return this->_mPoint;
}

void LayerSurface::point(DLM_POSITION point)
{
	this->_mPoint = point;
}

DLM_UNITVECTOR LayerSurface::normal()
{
	return this->_mNormal;
}

void LayerSurface::normal(DLM_UNITVECTOR normal)
{
	this->_mNormal = normal;
}

void LayerSurface::angle(double value)
{
	this->_mAngle = value;
}

double LayerSurface::angle()
{
	return this->_mAngle;
}

void LayerSurface::initial_surface(bool flag)
{
	this->_bInitialSurface = flag;
}

bool LayerSurface::is_initial_surface()
{
	return this->_bInitialSurface;
}


void LayerSurface::owner(LayerBody* owner)
{
	this->_pOwner = owner;
}

LayerBody* LayerSurface::owner()
{
	return this->_pOwner;
}

void LayerSurface::direction(Direction stype)
{
	this->_eSurfDir = stype;
}

Direction LayerSurface::direction()
{
	return this->_eSurfDir;
}

void LayerSurface::id(int idnum)
{
	this->_mId = idnum;
}

int LayerSurface::id()
{
	return this->_mId;
}

void LayerSurface::pair(LayerSurface* pair)
{
	this->_pSurfPair = pair;
}

LayerSurface* LayerSurface::pair()
{
	return this->_pSurfPair;
}

void LayerSurface::pair_clear()
{
	this->_pSurfPair = nullptr;
}

void LayerSurface::stiffener_gen(bool val)
{
	this->_bStiffenerGenerated = val;
}

bool LayerSurface::is_stiffener_gen()
{
	return this->_bStiffenerGenerated;
}

void LayerSurface::stiffener_paired(bool val)
{
	this->_bStiffenerPaired = val;
}

bool LayerSurface::is_stiffener_paired()
{
	return this->_bStiffenerPaired;
}

void LayerSurface::created_from(LayerSurface* ls)
{
	this->_pCreatedFrom = ls;
}

void LayerSurface::created_from(LayerSurface& ls)
{
	this->_pCreatedFrom = &ls;
}

LayerSurface* LayerSurface::created_from()
{
	return this->_pCreatedFrom;
}
