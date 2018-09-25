#include "MBBody.h"


MBBody::MBBody()
{
	this->init_vars();
}

MBBody::MBBody(const MBBody& rhs)
{
	this->init_vars();
	this->copy_vars(rhs, *this);
}

MBBody::~MBBody()
{
	this->delete_vars();
}

MBBody& MBBody::operator=(const MBBody& rhs)
{
	if (this != &rhs)
	{
		this->copy_vars(rhs, *this);
	}
	return *this;
}

void MBBody::copy(MBBody& lhs)
{
	this->copy_vars(*this, lhs);
}

void MBBody::init_vars()
{
	// Objects which belong to 3rd party solid modeling libraries are always set to NULL
	this->_pBody = NULL;
	this->_pName = NULL;
}

void MBBody::delete_vars()
{
	// Objects which belong to 3rd party solid modeling libraries are always set to NULL
	this->_pBody = NULL;
	this->_pName = NULL;
}

void MBBody::copy_vars(const MBBody& rhs, MBBody& lhs)
{
	lhs._pBody = rhs._pBody;
	lhs._pName = rhs._pName;
}

DLM_BODYP MBBody::body()
{
	return this->_pBody;
}

void MBBody::body(DLM_BODYP body)
{
	this->_pBody = body;
}

char* MBBody::name()
{
	return this->_pName;
}

void MBBody::name(const char* name)
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

void MBBody::error_handler(int errnum)
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
