// ----------------------------------------------------------------------------
// situbeRenderMult.h: An extension to CSitubeRender, serving precise
//						interaction synchronization in CMitubeRenderView and 
//						CMitubeRenderWindow. For example: 
//
//						support class friendship with these two classes and; 
//						allow deliberate control of the inner logic control;
//						more to go 
//
// Creation : June. 14th 2011
//
// Revision:
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef  _SITUBERENDERMULT_H_

#include "situbeRender.h"

class CMitubeRenderView;
class CMitubeRenderWindow;

class CSitubeRenderMult: public CSitubeRender {
public:
	friend class CMitubeRenderView;
	friend class CMitubeRenderWindow;
	CSitubeRenderMult(int argc, char **argv);
	virtual ~CSitubeRenderMult();

	int loadGeometry();

	int mainstay();

protected:
	void _realkeyResponse(unsigned char key, int x, int y);

	void _calibrateLineGeometry(const point_t & minpt, const point_t & maxpt);
	void _calibrateTubeGeometry(const point_t & minpt, const point_t & maxpt);

private:
	// expedient variables for the boxPang study of multiple view contrast
	int m_nRunNum;
	bool m_bTaskDone;
};

#endif // _SITUBERENDERMULT_H_

/* set ts=4 sts=4 tw=80 sw=4 */

