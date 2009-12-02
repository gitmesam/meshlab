/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2007                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#ifndef FILTERDIRTPLUGIN_H
#define FILTERDIRTPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QString>

#include <meshlab/meshmodel.h>
#include <meshlab/interfaces.h>
#include<vector>
#include<vcg/simplex/vertex/base.h>
#include<vcg/simplex/face/base.h>
#include<vcg/complex/trimesh/base.h>


using namespace vcg;

class DustEdge;//Never used
class DustVertex;//Never used
class DustFace;

class DustVertex : public VertexSimp2<DustVertex, DustEdge, DustFace> {};
class DustFace:public FaceSimp2< DustVertex, DustEdge, DustFace,face::Normal3f>{};
class DustMesh:public tri::TriMesh< std::vector<DustVertex>,std::vector<DustFace> >{};



class FilterDirt : public QObject, public MeshFilterInterface
{
	Q_OBJECT
	Q_INTERFACES(MeshFilterInterface)

	public:
		enum {FP_DIRT} ;
		
        FilterDirt();
        ~FilterDirt(){};

        virtual QString filterName(FilterIDType filter) const;
        virtual QString filterInfo(FilterIDType filter) const;
        virtual int getRequirements(QAction *);
        virtual bool autoDialog(QAction *) {return true;}
        virtual void initParameterSet(QAction* filter,MeshModel &,RichParameterSet &){};
        virtual void initParameterSet(QAction *,MeshDocument &/*m*/, RichParameterSet & /*parent*/);
        virtual bool applyFilter(QAction*  filter, MeshDocument &md, RichParameterSet & par, vcg::CallBackPos *cb);
        virtual bool applyFilter(QAction * /*filter */, MeshModel &, RichParameterSet & /*parent*/, vcg::CallBackPos *) { assert(0); return false;} ;
        virtual FilterClass getClass(QAction *);

};


#endif
