#ifndef LAYERMOLD_H
#define LAYERMOLD_H

#include "APIConfig.h"
#include "MBBody.h"
// Export file is generated by CMake's GenerateExportHeader module
#include "modelbuilder_export.h"


// Forward declarations
class Layer;

// Defines the structure of the mold
class MODELBUILDER_EXPORT LayerMold : public MBBody
{
public:

	/**
	 * \brief Default constructor.
	 */
	LayerMold();

	/**
	 * \brief Copy constructor.
	 *
	 * By default, this copy constructor does deep copy
	 * \param rhs object to be copied
	 */
	LayerMold(const LayerMold& rhs);

	/**
	 * \brief Default destructor.
	 */
	~LayerMold();

	/**
	 * \brief Copy assignment operator.
	 *
	 * By default, this copy assignment operator does deep copy
	 * \param rhs object to be copied
	 */
	LayerMold& operator=(const LayerMold& rhs);

	/**
	 * \brief Copies the layer mold to lhs.
	 * \param lhs new object
	 */
	void copy(LayerMold& lhs);

	/**
	 * \brief Gets the owner of this LayerMold object.
	 * \return pointer to the owner Layer object
	 */
	const Layer* owner();

	/**
	 * \brief Sets the owner of this LayerMold object.
	 * \param owner_ptr pointer to a Layer object
	 */
	void owner(Layer *owner_ptr);

	/**
	 * \brief Gets the intented direction of this LayerMold object.
	 * \return direction of the mold
	 */
	Direction direction();

	/**
	 * \brief Sets the intented direction of this LayerMold object.
	 * \param mold_dir direction of the mold
	 */
	void direction(Direction mold_dir);

	/**
	 * \brief Gets the stored sheet body representation of the mold.
	 * \return a BODY object
	 */
	DLM_BODYP body();

	/**
	 * \brief Sets the stored sheet body representation of the mold.
	 * \param mold a BODY object
	 */
	void body(DLM_BODYP body);

	/**
	* \brief Gets the LayerMold name.
	* \return name of the LayerMold
	*/
	char* name();

	/**
	* \brief Sets the LayerMold name.
	* \param name new name of the LayerMold
	*/
	void name(const char* name);

	/**
	 * \brief Sets "generated from stiffener" flag
	 * \param val flag value
	 */
	void stiffener_gen(bool val);

	/**
	 * \brief Returns "generated from stiffener" flag
	 * \return if this mold is generated by stiffener imprinting then returns TRUE, otherwise FALSE
	 */
	bool is_stiffener_gen();

private:
	Layer* _pOwner; /**< Pointer to the owner of this mold object */
	Direction _eDirection; /**< Direction of this mold, i.e. ORIG or OFFSET */
	bool _bStiffenerGenerated; /**< Stores "generated from stiffener" information */

	// DLM_BODYP defined in base class
	//DLM_BODYP _pBody; /**< Representation of the mold as a sheet body */

	void init_vars();
	void delete_vars();
	void copy_vars(const LayerMold& rhs, LayerMold& lhs);
};

#endif // !LAYERMOLD_H