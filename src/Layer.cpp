#include "Layer.h"


Layer::Layer()
{
	this->init_vars();
	// Setting the default layer name like this works better on the Python side
	this->name("UnnamedLayer");
}

Layer::Layer(const Layer& rhs)
{
	this->init_vars();
	// Setting the default layer name like this works better on the Python side
	this->name("UnnamedLayer");
	this->copy_vars(rhs, *this);
}

Layer::~Layer()
{
	this->delete_vars();
}

Layer& Layer::operator=(const Layer& rhs)
{
	// Check for self assignment
	if (this != &rhs)
	{
		this->copy_vars(rhs, *this);
	}
	return *this;
}

void Layer::copy(Layer& lhs)
{
	this->copy_vars(*this, lhs);
}

void Layer::init_vars()
{
	this->_mPosOrig = 0.0;
	this->_mPosOffset = 0.0;
	this->_pName = nullptr;
	// Initialize the layer ID with an unreachable value so that we can check if it is modified or not
	this->_mId = std::numeric_limits<int>::max();
	this->_mLayup = 0;
	this->_eType = LayerType::LAMINA;
	this->_pBodyList = nullptr;
	this->_mBodyListSize = 0;
	this->_pMoldList = nullptr;
	this->_mMoldListSize = 0;
	this->_pPairOrig = nullptr;
	this->_pPairOffset = nullptr;
	this->next_lb_id = 0;
	this->_mDelamRefMold=nullptr;
}

void Layer::delete_vars()
{
	if (this->_pName != nullptr)
	{
		delete[] this->_pName;
		this->_pName = nullptr;
	}

	if (this->_pBodyList != nullptr)
	{
		delete[] this->_pBodyList;
		this->_pBodyList = nullptr;
		this->_mBodyListSize = 0;
	}

	if (this->_pMoldList != nullptr)
	{
		delete[] this->_pMoldList;
		this->_pMoldList = nullptr;
		this->_mMoldListSize = 0;
	}

	this->_pPairOrig = nullptr;
	this->_pPairOffset = nullptr;
}

void Layer::copy_vars(const Layer& rhs, Layer& lhs)
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

	lhs._mPosOrig = rhs._mPosOrig;
	lhs._mPosOffset = rhs._mPosOffset;
	lhs._mId = rhs._mId;
	lhs._mLayup = rhs._mLayup;

	// Copy body list
	if (lhs._pBodyList != nullptr)
	{
		delete[] lhs._pBodyList;
		lhs._pBodyList = nullptr;
	}
	lhs._mBodyListSize = rhs._mBodyListSize;
	lhs._pBodyList = new LayerBody*[lhs._mBodyListSize];
	std::copy(rhs._pBodyList, rhs._pBodyList + rhs._mBodyListSize, lhs._pBodyList);

	// Copy mold list
	if (lhs._pMoldList != nullptr)
	{
		delete[] lhs._pMoldList;
		lhs._pMoldList = nullptr;
	}
	lhs._mMoldListSize = rhs._mMoldListSize;
	lhs._pMoldList = new LayerMold*[lhs._mMoldListSize];
	std::copy(rhs._pMoldList, rhs._pMoldList + rhs._mMoldListSize, lhs._pMoldList);
	
	lhs._eType = rhs._eType;
	lhs._eDirection = rhs._eDirection;
	lhs._pPairOrig = rhs._pPairOrig;
	lhs._pPairOffset = rhs._pPairOffset;
	lhs.next_lb_id = rhs.next_lb_id;
	lhs._mDelamRefMold = rhs._mDelamRefMold;
}

double Layer::thickness()
{
	return std::abs(this->_mPosOffset - this->_mPosOrig);
}

double Layer::position_orig()
{
	return this->_mPosOrig;
}

double Layer::position_offset()
{
	return this->_mPosOffset;
}

void Layer::position(double pos_at_orig, double pos_at_offset)
{
	this->_mPosOrig = pos_at_orig;
	this->_mPosOffset = pos_at_offset;
}

char* Layer::name()
{
	return this->_pName;
}

void Layer::name(const char* name)
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

	// Update LayerBody names
	this->update_bodynames();
}

int Layer::id()
{
	return this->_mId;
}

void Layer::id(int value)
{
	this->_mId = value;
}

int Layer::layup()
{
	return this->_mLayup;
}

void Layer::layup(int layup)
{
	this->_mLayup = layup;
}

void Layer::type(LayerType type)
{
	this->_eType = type;
}

LayerType Layer::type()
{
	return this->_eType;
}

void Layer::direction(Direction direction)
{
	this->_eDirection = direction;
}

Direction Layer::direction()
{
	return this->_eDirection;
}

void Layer::bodynames(char**& namelist, int& namelist_size)
{
	namelist_size = this->_mBodyListSize;
	namelist = new char*[namelist_size];
	int i = 0;
	for (auto lb_in : *this)
	{
		namelist[i] = strdup(lb_in->name());
		i++;
	}
}

void Layer::update_bodynames()
{
	if (this->name() != nullptr)
	{
		std::string currentLayerName = std::string(this->name());
		for (int i = 0; i < this->_mBodyListSize; i++)
		{
			std::string currentBodyName = std::string(this->_pBodyList[i]->name());
			std::string newBodyName = currentLayerName + currentBodyName;
			this->_pBodyList[i]->name(newBodyName.c_str());
		}
	}
}

void Layer::update_owners()
{
	for (int i = 0; i < this->_mBodyListSize; i++)
	{
		// Update LayerBody owner
		this->_pBodyList[i]->owner(this);

		// Update LayerSurface owner
		LayerSurface** ls = this->_pBodyList[i]->list();
		int ls_size = this->_pBodyList[i]->size();
		for (int j = 0; j < ls_size; j++)
		{
			ls[j]->owner(this->_pBodyList[i]);
		}
	}
}

void Layer::add_body(LayerBody *elem)
{
	if (this->_pBodyList != nullptr)
	{
		// Allocate a new body list
		LayerBody** new_list = new LayerBody*[this->_mBodyListSize + 1];
		std::copy(this->_pBodyList, this->_pBodyList + this->_mBodyListSize, new_list);
		new_list[this->_mBodyListSize] = elem;
		this->_mBodyListSize += 1;

		// Delete existing body list
		delete[] this->_pBodyList;
		this->_pBodyList = nullptr;

		// Copy new list over the body list fields
		this->_pBodyList = new LayerBody*[this->_mBodyListSize];
		std::copy(new_list, new_list + this->_mBodyListSize, this->_pBodyList);
		delete[] new_list;
		new_list = nullptr;
	}
	else
	{
		this->_mBodyListSize += 1;
		this->_pBodyList = new LayerBody*[this->_mBodyListSize];
		this->_pBodyList[0] = elem;
	}
	this->next_lb_id++;
}

class LayerBody** Layer::list()
{
	return this->_pBodyList;
}

LayerBody** Layer::begin()
{
	return &this->_pBodyList[0];
}

LayerBody** Layer::end()
{
	return &this->_pBodyList[this->_mBodyListSize];
}

LayerBody* Layer::at(int idx)
{
	if (idx < 0 || idx >= this->_mBodyListSize)
		throw std::out_of_range("Index value exceeds container size");
	return this->_pBodyList[idx];
}

int Layer::size()
{
	return this->_mBodyListSize;
}

void Layer::clear()
{
	if (this->_pBodyList != nullptr)
	{
		delete[] this->_pBodyList;
		this->_pBodyList = nullptr;
	}
	this->_mBodyListSize = 0;
	// Reset LayerBody counter
	this->next_lb_id = 0;
}

void Layer::remove(int idx)
{
	// User input checking
	if (idx < 0 || idx >= this->_mBodyListSize)
	{
		std::cout << "Index value is out of range!" << std::endl;
		return;
	}
	
	// Create a temporary pointer array for storing new LayerBody list
	int sz_new = this->_mBodyListSize - 1;
	LayerBody** list_new = new LayerBody*[sz_new];
	int list_new_cnt = 0;

	// Update the new pointer array
	for (int i = 0; i < this->_mBodyListSize; i++)
	{
		if (i != idx)
		{
			list_new[list_new_cnt] = this->_pBodyList[i];
			list_new_cnt++;
		}
	}

	// Delete the current body list and update it with the new one
	delete[] this->_pBodyList;
	this->_pBodyList = nullptr;
	this->_pBodyList = new LayerBody*[sz_new];
	std::copy(list_new, list_new + sz_new, this->_pBodyList);
	this->_mBodyListSize = sz_new;

	// Deallocate the temoorary body list pointer array
	delete[] list_new;
	list_new = nullptr;
}

void Layer::print_bodylist(bool extra_information)
{
	for (int i = 0; i < this->_mBodyListSize; i++)
	{
		std::cout << "LayerBody " << i << ": " << this->_pBodyList[i]->name() << std::endl;

		// Print extra information if the flag is set to TRUE
		if (extra_information)
		{
			std::cout << "List of LayerSurface objects contained in this LayerBody:" << std::endl;
			for (auto ls : *this->_pBodyList[i])
			{
				std::cout << "LayerSurface " << ls->id();
				std::string bctype_str;
				switch (ls->delam_type())
				{
				case DelaminationType::NODELAM: bctype_str = "No Delamination"; break;
				case DelaminationType::NOMODEL: bctype_str = "No Model"; break;
				case DelaminationType::CONTACT: bctype_str = "Contact"; break;
				case DelaminationType::COHESIVE: bctype_str = "Cohesive"; break;
				case DelaminationType::TIE: bctype_str = "Tie"; break;
				//case DelaminationType::CONTACT_OR_NOMODEL: bctype_str = "Contact or No Model"; break;
				default: bctype_str = "Undefined Type";
				}
				std::cout << " (" << bctype_str << ") :";
				std::cout << std::setprecision(3) << "[" << ls->point_coords().x() << ", " << ls->point_coords().y() << ", " << ls->point_coords().z() << "]" << std::endl;
			}
		}
	}
}

void Layer::add_mold(LayerMold *elem)
{
	if (this->_pMoldList != nullptr)
	{
		// Allocate a new body list
		LayerMold** new_list = new LayerMold*[this->_mMoldListSize + 1];
		std::copy(this->_pMoldList, this->_pMoldList + this->_mMoldListSize, new_list);
		new_list[this->_mMoldListSize] = elem;
		this->_mMoldListSize += 1;

		// Delete existing body list
		delete[] this->_pMoldList;
		this->_pMoldList = nullptr;

		// Copy new list over the body list fields
		this->_pMoldList = new LayerMold*[this->_mMoldListSize];
		std::copy(new_list, new_list + this->_mMoldListSize, this->_pMoldList);
		delete[] new_list;
		new_list = nullptr;
	}
	else
	{
		this->_mMoldListSize += 1;
		this->_pMoldList = new LayerMold*[this->_mMoldListSize];
		this->_pMoldList[0] = elem;
	}
}

LayerMold** Layer::list_mold()
{
	return this->_pMoldList;
}

int Layer::size_mold()
{
	return this->_mMoldListSize;
}

void Layer::clear_mold()
{
	if (this->_pMoldList != nullptr)
	{
		delete[] this->_pMoldList;
		this->_pMoldList = nullptr;
	}
	this->_mMoldListSize = 0;
}

bool Layer::is_bonded(Direction bdir)
{
	if (bdir == Direction::OFFSET && this->_pPairOffset != nullptr)
		return true;
	else if (bdir == Direction::ORIG && this->_pPairOrig != nullptr)
		return true;
	else
		return false;
}

Layer* Layer::bond_pair(Direction bond_dir)
{
	if (bond_dir == Direction::OFFSET)
		return this->_pPairOffset;
	else if (bond_dir == Direction::ORIG)
		return this->_pPairOrig;
	else
		return nullptr;
}

void Layer::bond_pair(Direction bond_dir, Layer* layer_pair)
{
	if (bond_dir == Direction::OFFSET)
		this->_pPairOffset = layer_pair;
	else if (bond_dir == Direction::ORIG)
		this->_pPairOrig = layer_pair;
	else
		std::cout << "ERROR: Set the direction correctly!" << std::endl;
	// TO-DO: Fix error message
}

LayerMold* Layer::delam_profile_ref()
{
	// Check if we have a correct mold
	if (!this->_mDelamRefMold || this->_mDelamRefMold->direction() == Direction::NODIR)
		return nullptr;
	return this->_mDelamRefMold;
}

void Layer::delam_profile_ref(LayerMold *orig_mold)
{
	this->_mDelamRefMold = orig_mold;
}
