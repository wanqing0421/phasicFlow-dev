/*------------------------------- phasicFlow ---------------------------------
      O        C enter of
     O O       E ngineering and
    O   O      M ultiscale modeling of
   OOOOOOO     F luid flow       
------------------------------------------------------------------------------
  Copyright (C): www.cemf.ir
  email: hamid.r.norouzi AT gmail.com
------------------------------------------------------------------------------  
Licence:
  This file is part of phasicFlow code. It is a free software for simulating 
  granular and multiphase flows. You can redistribute it and/or modify it under
  the terms of GNU General Public License v3 or any other later versions. 
 
  phasicFlow is distributed to help others in their research in the field of 
  granular and multiphase flows, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

-----------------------------------------------------------------------------*/
#ifndef __pointFlag_hpp__
#define __pointFlag_hpp__


#include "phasicFlowKokkos.hpp"
#include "domain.hpp"
#include "scatteredFieldAccess.hpp"

namespace pFlow
{

template<typename ExecutionSpace>
class pointFlag
{
	enum Flag: uint8
	{
		DELETED 	= 1, 	//10000000
		INTERNAL 	= 2,	//00000001
		LEFT 		= 4,	//00000010
		RIGHT 		= 8,    //00000100
		BOTTOM 		= 16,	//00001000
		TOP 		= 32,	//00010000
		REAR 		= 64, 	//00100000
		FRONT 		= 128	//01000000
	};

	using execution_space 	= ExecutionSpace;

	using memory_space 		= typename execution_space::memory_space;

	using viewType 			= ViewType1D<uint8, memory_space>;

	using device_type 		= typename viewType::device_type;

protected:
	
	viewType 	flags_;

	uint32 		numActive_ 		= 0;

	rangeU32 	activeRange_ 	= {0,0};

	bool 		isAllActive_	= false;

	uint32 		nLeft_ 	= 0;

	uint32 		nRight_ = 0;

	uint32 		nBottom_= 0;

	uint32 		nTop_ 	= 0;

	uint32 		nRear_ 	= 0;

	uint32 		nFront_ = 0;

	//- Protected methods 

	

public:

	friend class internalPoints;

	pointFlag()
	{}

	pointFlag(uint32 capacity, uint32 start, uint32 end )
	:
		flags_("pointFlag", capacity ),
		numActive_(end-start),
		activeRange_(start, end),
		isAllActive_(true)
	{
		fill(flags_, 0, capacity, static_cast<uint8>(Flag::DELETED));
		fill(flags_, activeRange_, static_cast<uint8>(Flag::INTERNAL));
	}

	pointFlag(
		viewType 	flags,
		uint32 		numActive,
		rangeU32 	activeRange,
		bool 		isAllActive)
	:
		flags_(flags),
		numActive_(numActive),
		activeRange_(activeRange),
		isAllActive_(isAllActive)
	{}

	pointFlag(const pointFlag&) = default;

	pointFlag& operator=(const pointFlag&) = default;

	pointFlag(pointFlag&&) = default;

	pointFlag& operator=(pointFlag&&) = default;

	~pointFlag() = default;	

	INLINE_FUNCTION_HD
	bool isAllActive()const
	{
		return isAllActive_;
	}

	INLINE_FUNCTION_HD
	const auto& activeRange()const
	{
		return activeRange_;
	}

	INLINE_FUNCTION_HD
	auto numActive()const
	{
		return numActive_;
	}

	INLINE_FUNCTION_HD
	auto leftSize()const
	{
		return nLeft_;
	}

	INLINE_FUNCTION_HD
	auto rightSize()const
	{
		return nRight_;
	}

	INLINE_FUNCTION_HD
	auto bottomSize()const
	{
		return nBottom_;
	}

	INLINE_FUNCTION_HD
	auto topSize()const
	{
		return nTop_;
	}

	INLINE_FUNCTION_HD
	auto rearSize()const
	{
		return nRear_;
	}

	INLINE_FUNCTION_HD
	auto frontSize()const
	{
		return nFront_;
	}


	INLINE_FUNCTION_HD
	bool operator()(uint32 i)const
	{
		return isActive(i);
	}

	INLINE_FUNCTION_HD
	bool isActive(uint32 i)const
	{
		return flags_[i] > DELETED;
	}

	INLINE_FUNCTION_HD
	bool isBoundary(uint32 i)const
	{
		return flags_[i] > INTERNAL;
	}

	INLINE_FUNCTION_HD
	bool isBoundary(uint8 flg)const
	{
		return flg > INTERNAL;
	}

	
	INLINE_FUNCTION_HD
	bool isLeft(uint8 flg)const
	{
		return (flg&LEFT) == LEFT;
	}

	INLINE_FUNCTION_HD
	bool isRight(uint8 flg)const
	{
		return (flg&&RIGHT) == RIGHT;
	}


	INLINE_FUNCTION_HD
	bool isBottom(uint8 flg)const
	{
		return (flg&&BOTTOM) == BOTTOM;
	}

	INLINE_FUNCTION_HD
	bool isTop(uint8 flg)const
	{
		return (flg&&TOP) == TOP;
	}

	INLINE_FUNCTION_HD
	bool isRear(uint8 flg)const
	{
		return (flg&&REAR) == REAR;
	}

	INLINE_FUNCTION_HD
	bool isFront(uint8 flg)const
	{
		return (flg&&FRONT) == FRONT;
	}

	template<typename ExeSpace>
	pointFlag<ExeSpace> clone()const
	{
		using newViewType = ViewType1D<uint8, typename ExeSpace::memory_space>;
		newViewType newFlags(
			"pointFlag",
			flags_.size());

		copy(newFlags, flags_);

		return pointFlag<ExeSpace>(
			newFlags,
			numActive_,
			activeRange_,
			isAllActive_);
	}

	/// @brief Loop over the active points and mark those out of the box
	/// and return number of deleted points 
	/// @param validBox the box whose inside is valid 
	/// @param points list of all points 
	/// @return number of deleted points 
	uint32 markOutOfBoxDelete(
		const box& validBox,
		ViewType1D<realx3, memory_space> points);

	
	bool deletePoints(
		scatteredFieldAccess<uint32, memory_space> points);
	
	/// @brief mark points based on their position in the domain.
	/// This should be the first method to be called when updating
	/// boundaries (step 1 of 2).
	/// @param dm the domain for which particles are tested 
	/// @param points list of points 
	/// @param leftLength neighbor length of the left face 
	/// @param rightLength neighbor length of the right face 
	/// @param bottomLength neighbor length of the bottom face 
	/// @param topLength neighbor length of the top face 
	/// @param rearLength neighbor length of the rear face 
	/// @param frontLength neighbor length of the front face 
	/// @return number of deleted points 
	uint32 markPointRegions(
		domain 								dm,
		ViewType1D<realx3, memory_space> 	points,
		real leftLength,
		real rightLength,
		real bottomLength,
		real topLength,
		real rearLength,
		real frontLength);

	
	/// @brief fill the lists for boundary faces. Lists keep the index 
	/// of particles in the neighborhood of the faces. This mehtod is 
	/// called after markPointRegions (step 2 of 2).
	/// @param leftList neighbor list of the left face 
	/// @param rightList neighbor list of the right face 
	/// @param bottomList neighbor list of the bottom face 
	/// @param topList neighbor list of the top face 
	/// @param rearList neighbor list of the rear face 
	/// @param frontList neighbor list of the front face 
	void fillNeighborsLists(
		ViewType1D<uint32, memory_space> leftList,
		ViewType1D<uint32, memory_space> rightList,
		ViewType1D<uint32, memory_space> bottomList,
		ViewType1D<uint32, memory_space> topList,
		ViewType1D<uint32, memory_space> rearList,
		ViewType1D<uint32, memory_space> frontList);

	uint32 markDelete(
		const plane& 		pln,
		ViewType1D<realx3, memory_space> 	points,
		ViewType1D<uint32, memory_space> 	indices,
		ViewType1D<uint32, memory_space> 	onOff);



};

using pFlagTypeDevice 	= pointFlag<DefaultExecutionSpace>;

using pFlagTypeHost 	= pointFlag<DefaultHostExecutionSpace>;

}

#include "pointFlagKernels.hpp"

#endif // __pointFlag_hpp__
