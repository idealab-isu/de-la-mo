/* 
 * File: CADmodeler_extends.i
 * Module: CADmodeler
 *
 * "extend" directives
 */

 %extend delamo::List< std::string >
 {
	std::string __getitem__(unsigned int idx) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in StringList::__getitem__()");
		return (*self)[idx];
	}

	void __setitem__(unsigned int idx, std::string val) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in StringList::__setitem__()");
		(*self)[idx]=val;
	}

	unsigned int __len__()
	{
		return self->size();
	}
 }

%extend delamo::List< FaceAdjacency *>
{
	FaceAdjacency * __getitem__(unsigned int idx) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in FAL::__getitem__()");
		return (*self)[idx];
	}

	void __setitem__(unsigned int idx, FaceAdjacency *val) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in FAL::__setitem__()");
		(*self)[idx]=val;
	}

	unsigned int __len__()
	{
		return self->size();
	}
}

%extend delamo::List< Layer *>
{
	Layer *__getitem__(unsigned int idx) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in LayerList::__getitem__()");
		return (*self)[idx];
	}

	void __setitem__(unsigned int idx, Layer *val) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in LayerList::__setitem__()");
		(*self)[idx]=val;
	}

	unsigned int __len__()
	{
		return self->size();
	}
}


%extend delamo::List< MBBody *>
{
	MBBody *__getitem__(unsigned int idx) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in MBBodyList::__getitem__()");
		return (*self)[idx];
	}

	void __setitem__(unsigned int idx, MBBody *val) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in MBBodyList::__setitem__()");
		(*self)[idx]=val;
	}

	unsigned int __len__()
	{
		return self->size();
	}
}


%extend delamo::List< LayerMold *>
{
	LayerMold *__getitem__(unsigned int idx) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in LayerMoldList::__getitem__()");
		return (*self)[idx];
	}

	void __setitem__(unsigned int idx, LayerMold *val) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in LayerMoldList::__setitem__()");
		(*self)[idx]=val;
	}

	unsigned int __len__()
	{
		return self->size();
	}
}



%extend Layer {  /* not delamo::Layer for some reason */
	LayerBody *__getitem__(unsigned int idx) throw (std::out_of_range)
	{
	  return self->at(idx);
	}


	unsigned int __len__()
	{
		return self->size();
	}
}
