#include "LayerMold.h"

LayerMold::LayerMold()
{
	this->init_vars();
}

LayerMold::LayerMold(const LayerMold& rhs)
{
	this->init_vars();
	this->copy_vars(rhs, *this);
}

LayerMold::~LayerMold()
{
	this->delete_vars();
}

LayerMold& LayerMold::operator=(const LayerMold& rhs)
{
	// Check for self assignment
	if (this != &rhs)
	{
		this->copy_vars(rhs, *this);
	}
	return *this;
}

void LayerMold::copy(LayerMold& lhs)
{
	this->copy_vars(*this, lhs);
}

void LayerMold::init_vars()
{
	this->_eDirection = Direction::NODIR;
	this->_pOwner = nullptr;
	// Objects which belong to 3rd party solid modeling libraries are always set to NULL
	this->_pBody = NULL;
	this->_bStiffenerGenerated = false;
}

void LayerMold::delete_vars()
{
	this->_pOwner = nullptr;
	// Objects which belong to 3rd party solid modeling libraries are always set to NULL
	this->_pBody = NULL;
}

void LayerMold::copy_vars(const LayerMold& rhs, LayerMold& lhs)
{
	lhs._pOwner = rhs._pOwner;
	lhs._eDirection = rhs._eDirection;
	lhs._pBody = rhs._pBody;
	lhs._pName = rhs._pName;
	lhs._bStiffenerGenerated = rhs._bStiffenerGenerated;
}

const Layer* LayerMold::owner()
{
	return this->_pOwner;
}

void LayerMold::owner(Layer *owner_ptr)
{
	// LayerMold only needs to know the memory address of its owner Layer
	this->_pOwner = owner_ptr;
}

Direction LayerMold::direction()
{
	return this->_eDirection;
}

void LayerMold::direction(Direction mold_dir)
{
	this->_eDirection = mold_dir;
}

DLM_BODYP LayerMold::body()
{
	return this->_pBody;
}

void LayerMold::body(DLM_BODYP body)
{
	this->_pBody = body;
}

char* LayerMold::name()
{
	return this->_pName;
}

void LayerMold::name(const char* name)
{
	if (name == nullptr)
		return;

	if (this->_pName != nullptr)
	{
		delete[] this->_pName;
		this->_pName = nullptr;
	}

	std::string namestr(name);
	this->_pName = new char[namestr.size() + 1];
	std::copy(namestr.c_str(), namestr.c_str() + namestr.size(), this->_pName);
	this->_pName[namestr.size()] = '\0';
}

void LayerMold::stiffener_gen(bool val)
{
	this->_bStiffenerGenerated = val;
}

bool LayerMold::is_stiffener_gen()
{
	return this->_bStiffenerGenerated;
}
