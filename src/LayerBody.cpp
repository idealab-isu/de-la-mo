#include "LayerBody.h"


LayerBody::LayerBody()
{
	this->init_vars();
}

LayerBody::LayerBody(const LayerBody& rhs)
{
	this->init_vars();
	this->copy_vars(rhs, *this);
}

LayerBody::~LayerBody()
{
	this->delete_vars();
}

LayerBody& LayerBody::operator=(const LayerBody& rhs)
{
	if (this != &rhs)
	{
		this->copy_vars(rhs, *this);
	}
	return *this;
}

void LayerBody::copy(LayerBody& lhs)
{
	this->copy_vars(*this, lhs);
}

void LayerBody::init_vars()
{
	this->_pName = nullptr;
	this->_pOwner = nullptr;
	this->_pSurfList = nullptr;
	this->_mSurfListSize = 0;
	this->next_ls_id = 0;
	// Objects which belong to 3rd party solid modeling libraries are always set to NULL
	this->_pBody = NULL;
}

void LayerBody::delete_vars()
{
	if (this->_pName != nullptr)
	{
		delete[] this->_pName;
		this->_pName = nullptr;
	}
	
	this->_pOwner = nullptr;

	if (this->_pSurfList != nullptr)
	{
		delete[] this->_pSurfList;
		this->_pSurfList = nullptr;
		this->_mSurfListSize = 0;
	}

	// Objects which belong to 3rd party solid modeling libraries are always set to NULL
	this->_pBody = NULL;
}

void LayerBody::copy_vars(const LayerBody& rhs, LayerBody& lhs)
{
	if (lhs._pName != nullptr)
	{
		delete[] this->_pName;
		this->_pName = nullptr;
	}
	if (rhs._pName != nullptr)
	{
		std::string rhsname(rhs._pName);
		this->_pName = new char[rhsname.size() + 1];
		std::copy(rhsname.c_str(), rhsname.c_str() + rhsname.size(), this->_pName);
		this->_pName[rhsname.size()] = '\0';
		rhsname.clear();
	}
	else
	{
		lhs._pName = nullptr;
	}


	lhs._pBody = rhs._pBody;
	
	// Copy surface list
	if (lhs._pSurfList != nullptr)
	{
		delete[] lhs._pSurfList;
		lhs._pSurfList = nullptr;
	}
	lhs._mSurfListSize = rhs._mSurfListSize;
	lhs._pSurfList = new LayerSurface*[lhs._mSurfListSize];
	std::copy(rhs._pSurfList, rhs._pSurfList + rhs._mSurfListSize, lhs._pSurfList);

	lhs._pOwner = rhs._pOwner;
	lhs.next_ls_id = rhs.next_ls_id;
}

char* LayerBody::name()
{
	return this->_pName;
}

void LayerBody::name(const char* name)
{
	if (name == nullptr)
		return;

	if (this->_pName != nullptr)
	{
		delete[] this->_pName;
		this->_pName = nullptr;
	}

	std::string namestr(name);
	this->_pName = new char[namestr.size()+1];
	std::copy(namestr.c_str(), namestr.c_str() + namestr.size(), this->_pName);
	this->_pName[namestr.size()] = '\0';
}

DLM_BODYP LayerBody::body()
{
	return this->_pBody;
}

void LayerBody::body(DLM_BODYP body)
{
	this->_pBody = body;
}

void LayerBody::add_surface(LayerSurface *elem)
{
	if (this->_pSurfList != nullptr)
	{
		// Allocate a new body list
		LayerSurface** new_list = new LayerSurface*[this->_mSurfListSize + 1];
		std::copy(this->_pSurfList, this->_pSurfList + this->_mSurfListSize, new_list);
		new_list[this->_mSurfListSize] = elem;
		this->_mSurfListSize += 1;

		// Delete existing body list
		delete[] this->_pSurfList;
		this->_pSurfList = nullptr;

		// Copy new list over the body list fields
		this->_pSurfList = new LayerSurface*[this->_mSurfListSize];
		std::copy(new_list, new_list + this->_mSurfListSize, this->_pSurfList);
		delete[] new_list;
		new_list = nullptr;
	}
	else
	{
		this->_mSurfListSize += 1;
		this->_pSurfList = new LayerSurface*[this->_mSurfListSize];
		this->_pSurfList[0] = elem;
	}
	// Increment LayerSurface counter
	this->next_ls_id++;
}

LayerSurface** LayerBody::begin()
{
	return &this->_pSurfList[0];
}

LayerSurface** LayerBody::end()
{
	return &this->_pSurfList[this->_mSurfListSize];
}

LayerSurface* LayerBody::at(int idx)
{
	if (idx < 0 || idx >= this->_mSurfListSize)
		throw std::out_of_range("Index value exceeds container size");
	return this->_pSurfList[idx];
}

int LayerBody::size()
{
	return this->_mSurfListSize;
}

void LayerBody::clear()
{
	if (this->_pSurfList != nullptr)
	{
		delete[] this->_pSurfList;
		this->_pSurfList = nullptr;
	}
	this->_mSurfListSize = 0;
	// Set the LayerSurface counter to zero
	this->next_ls_id = 0;
}


void LayerBody::owner(Layer* owner)
{
	this->_pOwner = owner;
}

Layer* LayerBody::owner()
{
	return this->_pOwner;
}

void LayerBody::error_handler(int errnum)
{
	if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_WARN)
	{
		std::string errstr;
		switch (errnum)
		{
		case 1: errstr = "WARN: Owner pointer is null!"; break;
		default: errstr = "WARN: Unknown error occurred!";
		}
		std::cout << errstr << std::endl;
	}
}

int LayerBody::face_id(DLM_FACEP face_query)
{
	// Query surface list for the given face
	for (int i = 0; i < this->_mSurfListSize; i++)
	{
		if (face_query == this->_pSurfList[i]->face())
			return i;
	}
	return -1;
}

LayerSurface** LayerBody::list()
{
	return this->_pSurfList;
}

void LayerBody::mold(LayerMold* new_mold)
{
	this->_mMold = *new_mold;
}

LayerMold* LayerBody::mold()
{
	if (NULL == this->_mMold.body())
		return nullptr;
	return &this->_mMold;
}
