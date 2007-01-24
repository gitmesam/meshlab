/****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2005                                                \/)\/    *
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
/****************************************************************************

//TODO CLEAN ME
//TODO better to vertex painting switch
//TODO bits instead of hashtables
//TODO PaintToolbox does not close
//TODO lines problem with percentual painting
----------- SOLVED: ------------
//TODO first paint hang problem -- PROBALLY SOLVED, not shure
//TODO trackball zbuffer problem OK
version 0.1 gfrei

****************************************************************************/
#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <meshlab/glarea.h>
#include "editpaint.h"
#include <stdio.h>
#include <wrap/gl/pick.h>
#include <limits>

#include<vcg/complex/trimesh/update/topology.h>
#include<vcg/complex/trimesh/update/bounding.h>
using namespace vcg;

EditPaintPlugin::EditPaintPlugin() {
	isDragging=false;
	paintbox=0;
	pixels=0;
	first=true;
	pressed=false;
	actionList << new QAction(QIcon(":/images/pinsel.png"),"Vertex painting", this);
	QAction *editAction;
	foreach(editAction, actionList)
	editAction->setCheckable(true);
	//qDebug("CONSTRUCTOR");
	//paintbox=new PaintToolbox("Vertex painting");
	//paintbox->setVisible(false);
	//worker= new PaintWorker();
	//worker->start(QThread::HighestPriority);
}

EditPaintPlugin::~EditPaintPlugin() {
	if (paintbox!=0) { delete paintbox; paintbox=0; }
	qDebug() << "~EditPaint" << endl;
}


QList<QAction *> EditPaintPlugin::actions() const {
	return actionList;
}

const QString EditPaintPlugin::Info(QAction *action) {
	if( action->text() != tr("Vertex painting") ) assert (0);
	return tr("Colorize the polygons of your mesh");
}

const PluginInfo &EditPaintPlugin::Info() {
	static PluginInfo ai; 
	ai.Date=tr(__DATE__);
	ai.Version = tr("0.001");
	ai.Author = ("Andreas Gfrei");
	return ai;
} 

void EditPaintPlugin::StartEdit(QAction * /*mode*/, MeshModel &m, GLArea * parent) {
	first=true;
	pressed=false;
	//qDebug() <<"startedit"<<  endl;
	tri::UpdateBounding<CMeshO>::Box(m.cm);
	if (paintbox==0) paintbox=new PaintToolbox(parent);
	//paintbox->setGeometry();
	paintbox->setVisible(true);
	//paintbox->diag=m.cm.bbox.Diag();
	//m.updateDataMask(MeshModel::MM_FACECOLOR);
	m.updateDataMask(MeshModel::MM_FACETOPO);
	//m.Enable(MeshModel::MM_FACECOLOR);
	//parent->setColorMode(vcg::GLW::CMPerVert);
	parent->getCurrentRenderMode().colorMode=vcg::GLW::CMPerVert;
	parent->mm->ioMask|=MeshModel::IOM_VERTCOLOR;
	parent->mm->ioMask|=MeshModel::IOM_VERTQUALITY;
	
	LastSel.clear();
	curSel.clear();
	parent->update();
}

void EditPaintPlugin::EndEdit(QAction * /*mode*/, MeshModel &/*m*/, GLArea * /*parent*/) {
	qDebug() <<"ENDEDIT"<<endl;
	if (paintbox!=0) { paintbox->setVisible(false); delete paintbox; paintbox=0; }
}

void EditPaintPlugin::mousePressEvent(QAction * ac, QMouseEvent * event, MeshModel &m, GLArea * gla) {
	//qDebug() << "pressStart" << endl;
	has_track=gla->isTrackBallVisible();
	gla->showTrackBall(false);
	LastSel.clear();
	first=true;
	pressed=true;
	isDragging = true;
	temporaneo.clear();
	start=event->pos();
	cur=start;
	prev=start;
	inverse_y=gla->curSiz.height()-cur.y();
	curr_mouse=event->button();
	
	pen.painttype=paintType();
	pen.backface=paintbox->getPaintBackface();
	pen.invisible=paintbox->getPaintInvisible();
	switch (paintType()) {
		case 1: { pen.radius=paintbox->getRadius()*0.5; } break;
		case 2: { pen.radius=paintbox->getRadius()*m.cm.bbox.Diag()*0.01*0.5; } break;
		case 3: { pen.radius=paintbox->getRadius()*m.cm.bbox.DimY()*0.01*0.5; } break;
		case 4: { pen.radius=paintbox->getRadius()*0.5; } break;
	}
	curSel.clear();

	switch (paintbox->paintUtensil()) {
		case FILL: {} break;
		case PICK: {} break;
		case PEN: {} break;
	}
	//qDebug() << "pressEnd" << endl;
}
  
void EditPaintPlugin::mouseMoveEvent(QAction *,QMouseEvent * event, MeshModel &/*m*/, GLArea * gla) {
	//qDebug() << "moveStart" << endl;
	switch (paintbox->paintUtensil()) {
		case FILL: { return; }
		case PICK: { return; }
		case PEN: {}
	}

	prev=cur;
	cur=event->pos();
	//pen.pos=cur;
	isDragging = true;
	// now the management of the update 
	//static int lastMouse=0;
	static int lastRendering=0;//clock();
	int curT = clock();
	//qDebug("mouseMoveEvent: curt %i last %i",curT,lastRendering);
	if(gla->lastRenderingTime() < 50 || (curT - lastRendering) > 1000 )
	{
		lastRendering=curT;
		gla->update();
	//qDebug("mouseMoveEvent: ----");
	}
	else {
		/*gla->makeCurrent();
		glDrawBuffer(GL_FRONT);
		DrawXORRect(gla,true);
		glDrawBuffer(GL_BACK);
		glFlush();*/
	}
	//qDebug() << "moveEnd" << endl;
}
  
void EditPaintPlugin::mouseReleaseEvent  (QAction *,QMouseEvent * event, MeshModel &m, GLArea * gla) {
	gla->showTrackBall(has_track);
	temporaneo.clear();
	gla->update();
	prev=cur;
	cur=event->pos();
	switch (paintbox->paintUtensil()) {
		case FILL: { return; }
		case PICK: { return; }
		case PEN: {}
	}
	isDragging=false;
}

inline void calcCoord(float x,float y,float z,double matrix[],double *xr,double *yr,double *zr) {
	*xr=x*matrix[0]+y*matrix[4]+z*matrix[8]+matrix[12];
	*yr=x*matrix[1]+y*matrix[5]+z*matrix[9]+matrix[13];	
	*zr=x*matrix[2]+y*matrix[6]+z*matrix[10]+matrix[14];
}

void getTranspose(double orig[],double inv[]) {
	inv[0]=orig[0]; inv[1]=orig[4]; inv[2]=orig[8];  inv[3]=orig[12];
	inv[4]=orig[1]; inv[5]=orig[5]; inv[6]=orig[9];  inv[7]=orig[13];
	inv[8]=orig[2]; inv[9]=orig[6]; inv[10]=orig[10];inv[11]=orig[14];
	inv[12]=orig[3];inv[13]=orig[7];inv[14]=orig[11]; inv[15]=orig[15];
}

inline int isIn(QPointF p0,QPointF p1,float dx,float dy,float radius) {
	//qDebug() << p0 << "  " << p1 << endl;
	float x0=(dx-p0.x());
	float y0=(dy-p0.y());
	float bla0=x0*x0+y0*y0;
	if (bla0<radius*radius) return 1;
	if (p0==p1) return 0;

	float x2=(p1.x()-p0.x());
	float y2=(p1.y()-p0.y());
	//double l=sqrt(x2*x2+y2*y2);
	float l_square=x2*x2+y2*y2;
	float r=(dx-p0.x())*(p1.x()-p0.x())+(dy-p0.y())*(p1.y()-p0.y());
	//r=r/(l*l);
	r=r/l_square;
	
	float px=p0.x()+r*(p1.x()-p0.x());
	float py=p0.y()+r*(p1.y()-p0.y());

	px=px-dx;
	py=py-dy;

	if (r>0 && r<1 && (px*px+py*py<radius*radius)) return 1;
	return 0;
}

inline bool pointInTriangle(QPointF p,QPointF a, QPointF b,QPointF c) {
	float fab=(p.y()-a.y())*(b.x()-a.x()) - (p.x()-a.x())*(b.y()-a.y());
	float fbc=(p.y()-c.y())*(a.x()-c.x()) - (p.x()-c.x())*(a.y()-c.y());
	float fca=(p.y()-b.y())*(c.x()-b.x()) - (p.x()-b.x())*(c.y()-b.y());
	if (fab*fbc>0 && fbc*fca>0) return true;
	return false;
}

inline bool isFront(QPointF a, QPointF b, QPointF c) {
	return (b.x()-a.x())*(c.y()-a.y())-(b.y()-a.y())*(c.x()-a.x())>0;
}

void EditPaintPlugin::DrawXORRect(MeshModel &m,GLArea * gla, bool doubleDraw) {
	int PEZ=18;
	if (paintType()==1) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0,gla->curSiz.width(),gla->curSiz.height(),0,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_XOR);
		glColor3f(1,1,1);
	
		QPoint mid= QPoint(cur.x(),/*gla->curSiz.height()-*/cur.y());
		if(doubleDraw) {
			glBegin(GL_LINE_LOOP);
			for (int lauf=0; lauf<PEZ; lauf++) {
				glVertex2f(mid.x()+sin(M_PI*(float)lauf/9.0)*pen.radius,mid.y()+cos(M_PI*(float)lauf/9.0)*pen.radius);
				//qDebug() << (mid.x()+sin(M_PI*(lauf/9.0)*10.0)) << " " << mid.y()+cos(M_PI*(lauf/9.0)*10.0) << endl;
			}
			glEnd();
		}
	
		glBegin(GL_LINE_LOOP);
		for (int lauf=0; lauf<PEZ; lauf++) {
			glVertex2f(mid.x()+sin(M_PI*(float)lauf/9.0)*pen.radius,mid.y()+cos(M_PI*(float)lauf/9.0)*pen.radius);
			//qDebug() << (mid.x()+sin(M_PI*(lauf/9.0)*10.0)) << " " << mid.y()+cos(M_PI*(lauf/9.0)*10.0) << endl;
		}
		glEnd();
	
		glDisable(GL_LOGIC_OP);

		/*glDisable(GL_DEPTH_TEST);
		glBegin(GL_TRIANGLES);
		Color4b co=paintbox->getColor(curr_mouse);
		glColor3f(co[0],co[1],co[2]);
		for (int lauf=0; lauf<PEZ; lauf++) {
			glVertex2f(mid.x(),mid.y());
			glVertex2f(mid.x()+sin(M_PI*(float)lauf/9.0)*pen.width.x()/2.0,mid.y()+cos(M_PI*(float)lauf/9.0)*pen.width.y()/2.0);
			glVertex2f(mid.x()+sin(M_PI*(float)(lauf+1)/9.0)*pen.width.x()/2.0,mid.y()+cos(M_PI*(float)(lauf+1)/9.0)*pen.width.y()/2.0);
		}
		glEnd();*/
	
	// Closing 2D
		glPopAttrib();
		glPopMatrix(); // restore modelview
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	} else {
		double dX, dY, dZ;
		double dX2, dY2, dZ2;
		
		updateMatrixes();
		
		QPoint mid= QPoint(cur.x(),gla->curSiz.height()-cur.y());
		gluUnProject ((double) mid.x(), mid.y(), 0.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
		gluUnProject ((double) mid.x(), mid.y(), 1.0, mvmatrix, projmatrix, viewport, &dX2, &dY2, &dZ2);


		glPushMatrix();
		glLoadIdentity();
		gluLookAt(dX,dY,dZ,dX2,dY2,dZ2,1,0,0);
		double mvmatrix2[16];
		glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix2);
		glPopMatrix();
	
		double tx,ty,tz;
		double tx2,ty2,tz2;
	
		double inv_mvmatrix[16];
	
		Matrix44d temp(mvmatrix2);
		Invert(temp);
		for (int lauf=0; lauf<16; lauf++) inv_mvmatrix[lauf]=temp[lauf/4][lauf%4];
		
		float radius=pen.radius;
		double a,b,c;
		double a2,b2,c2;

		PEZ=56;
		int STEPS=60;
		float diag=m.cm.bbox.Diag()*(-7);
		QPoint circle_points[PEZ];

		glPushAttrib(GL_ENABLE_BIT);
		//glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_XOR);
		glColor3f(1,1,1);

		glBegin(GL_LINES);
		for (int lauf=0; lauf<PEZ; lauf++) {
			calcCoord(sin(M_PI*(double)lauf/(PEZ/2))*radius,cos(M_PI*(double)lauf/(PEZ/2))*radius,diag,inv_mvmatrix,&tx,&ty,&tz);
			//glVertex3f(tx,ty,tz);
			gluProject(tx,ty,tz,mvmatrix,projmatrix,viewport,&a,&b,&c);

			calcCoord(sin(M_PI*(double)lauf/(PEZ/2))*radius,cos(M_PI*(double)lauf/(PEZ/2))*radius,0,inv_mvmatrix,&tx2,&ty2,&tz2);
			//glVertex3f(tx2,ty2,tz2);
			gluProject(tx2,ty2,tz2,mvmatrix,projmatrix,viewport,&a2,&b2,&c2);

			double da=(a-a2)/(double)STEPS;
			double db=(b-b2)/(double)STEPS;
			double dc=(c-c2)/(double)STEPS;
			double pix_x=a2;
			double pix_y=b2;
			double pix_z=c2;
			//circle_points[lauf]=QPoint(a2,old_size.y()-b2);
			//qDebug() <<"dabc: "<< da << " "<< db << " "<<dc << " abc: "<<a<<" "<<b<<" "<<c<< " a2b2c2: "<<a2<<" "<<b2<<" "<<c2<<endl;
			for (int lauf2=0; lauf2<STEPS; lauf2++) {
				pix_x+=da;
				pix_y+=db;
				pix_z+=dc;
				double inv_yy=gla->curSiz.height()-pix_y;
				//circle_points[lauf]=QPoint(pix_x,inv_yy);
				//qDebug() << pix_x << " "<<pix_y << endl;
				//if (pix_z<=0 || pix_z>=1)qDebug() <<"OK: "<< pix_x << " "<<pix_y <<" pix_z: "<< pix_z<<" zz: "<<endl;
					
				if ((int)pix_x>=0 && (int)pix_x<gla->curSiz.width() && (int)pix_y>=0 && (int)pix_y<gla->curSiz.height()) {
					double zz=(GLfloat)pixels[(int)(((int)pix_y)*gla->curSiz.width()+(int)pix_x)];
					if (zz<0.99999 && zz<pix_z /*&& pix_z<1*/ && pix_z>0) {
						circle_points[lauf]=QPoint(pix_x,inv_yy);
						//lauf2=1000;
						break;
					}
				}
				if (lauf2==STEPS-1 /*|| pix_z>=1*/) { circle_points[lauf]=QPoint(pix_x,inv_yy); break; }
			}

		}

		glEnd();
		glDisable(GL_COLOR_LOGIC_OP);
		glPopAttrib();
	

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0,gla->curSiz.width(),gla->curSiz.height(),0,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		//Color4b co=paintbox->getColor(curr_mouse);
		//qDebug() << co[0] << " " << co[1] << " " << co[2] << endl;
		glBegin(GL_TRIANGLES);
		glColor4d(1,1,1,0.13);
		for (int lauf=0; lauf<PEZ; lauf++) {
			glVertex2f(mid.x(),gla->curSiz.height()-mid.y());
			glVertex2f(circle_points[lauf].x(),circle_points[lauf].y());
			glVertex2f(circle_points[(lauf+1)%PEZ].x(),circle_points[(lauf+1)%PEZ].y());
		} 
		glEnd();
		glDisable(GL_BLEND);
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_XOR);
		glColor3f(1,1,1);
		glBegin(GL_LINE_STRIP);
		for (int lauf=0; lauf<PEZ; lauf++) {
			glVertex2f(circle_points[lauf].x(),circle_points[lauf].y());
		} glVertex2f(circle_points[0].x(),circle_points[0].y());
		/*if(doubleDraw) {
			for (int lauf=0; lauf<PEZ; lauf++) {
				glVertex2f(circle_points[lauf].x(),circle_points[lauf].y());
			} glVertex2f(circle_points[0].x(),circle_points[0].y());
		}*/
		glEnd();
		glDisable(GL_COLOR_LOGIC_OP);
		glPopAttrib();
		glPopMatrix(); // restore modelview
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

}

//    (Cx-Ax)(Bx-Ax) + (Cy-Ay)(By-Ay)
//r = ------------------------------
//                  L²


int EditPaintPlugin::paintType() { return paintbox->paintType(); }

void getInternFaces(MeshModel & m,vector<CMeshO::FacePointer> *actual,vector<CMeshO::VertexPointer> * risult, 
	GLArea * gla,Penn &pen,QPoint &cur, QPoint &prev, GLfloat * pixels,
	double mvmatrix[16],double projmatrix[16],int viewport[4]) {


	QHash <CFaceO *,CFaceO *> selezionati;
	QHash <CVertexO *,CVertexO *> sel_vert;
	vector<CMeshO::FacePointer>::iterator fpi;
	vector<CMeshO::FacePointer> temp_po;

	if (actual->size()==0) {
		CMeshO::FaceIterator fi;
        	for(fi=m.cm.face.begin();fi!=m.cm.face.end();++fi) 
            	if(!(*fi).IsD()) {
			temp_po.push_back((&*fi));
		}
	} else
	for(fpi=actual->begin();fpi!=actual->end();++fpi) {
		temp_po.push_back(*fpi);
	}

	actual->clear();

	QPointF mid=QPointF(cur.x(),gla->curSiz.height()-  cur.y());
	QPointF mid_prev=QPointF(prev.x(),gla->curSiz.height()-  prev.y());
	QPointF p[3];
	QPointF z[3];
	double tx,ty,tz;

	bool backface=pen.backface;
	bool invisible=pen.invisible;

	//qDebug() << "bf: "<<backface << " inv: "<<invisible << endl;

	if (pen.painttype==1) { /// PIXEL 
		for (int lauf2=0; lauf2<temp_po.size(); lauf2++) {
			CFaceO * fac=temp_po.at(lauf2);
			bool intern=false;
	
			for (int lauf=0; lauf<3; lauf++) {
				gluProject((fac)->V(lauf)->P()[0],(fac)->V(lauf)->P()[1],(fac)->V(lauf)->P()[2],mvmatrix,projmatrix,viewport,&tx,&ty,&tz);
				p[lauf]=QPointF(tx,ty);
				//qDebug() << "zzz: "<<(int)(((int)ty)*old_size.x()+(int)tx)<<" t: "<<tx<<" "<<ty<<" "<<tz<<endl;
				if (tx>=0 && tx<viewport[2] && ty>=0 && ty<viewport[3])
					z[lauf]=QPointF(tz,(GLfloat)pixels[(int)(((int)ty)*viewport[2]+(int)tx)]);
				else z[lauf]=QPoint(1,0);
				//qDebug() << "zzz_ende"<<endl;
				//float zz;
   				//glReadPixels( (int)tx, /*viewport[3]-*/(int)ty, 1, 1,GL_DEPTH_COMPONENT, GL_FLOAT, &zz );
				//qDebug () << "?? "<< /*viewport[3]-*/(int)ty<<"  "<<old_size.y()-(int)ty<< endl;
				//qDebug() << tx <<" "<<ty<<" z:"<<(float)tz<<" buf:"<<(GLfloat)pixels[(int)((/*old_size.y()-*/(int)ty)*old_size.x()+(int)tx)]
				//	<<" buf2: "<<zz<<endl;
			}
	
			if (backface || isFront(p[0],p[1],p[2])) {
				for (int lauf=0; lauf<3; lauf++) if (invisible || (z[lauf].x()<=z[lauf].y()+0.005)){
					tx=p[lauf].x();
					ty=p[lauf].y();
					if (isIn(mid,mid_prev,tx,ty,pen.radius)==1) {
						intern=true;
						if (!sel_vert.contains(fac->V(lauf))) {
							risult->push_back(fac->V(lauf));
							sel_vert.insert(fac->V(lauf),fac->V(lauf));
							//qDebug() << tx << " " << ty << " " << tz <<"   "<< mid<<endl;
						}
					}
				}
				if (!intern && (pointInTriangle(mid,p[0],p[1],p[2]) || pointInTriangle(mid_prev,p[0],p[1],p[2]))) {
					intern=true;
				}
			}
			if (intern && ! selezionati.contains(fac)) {
				selezionati.insert((fac),(fac));
				actual->push_back(fac);
				for (int lauf=0; lauf<3; lauf++) {
					CFaceO * nf=(fac)->FFp(lauf);
					//qDebug () << "nf" << nf->V(0)->P()[0] << endl;
					if (!selezionati.contains(nf)) {
						//actual->push_back(nf);
						temp_po.push_back(nf);
					}
				}
			}
		}

	} else { /// PERCENTUAL
		double dX, dY, dZ;
		double dX2, dY2, dZ2;
		gluUnProject ((double) mid.x(), mid.y(), 0.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
		gluUnProject ((double) mid.x(), mid.y(), 1.0, mvmatrix, projmatrix, viewport, &dX2, &dY2, &dZ2);
		glPushMatrix();
		glLoadIdentity();
		gluLookAt(dX,dY,dZ,dX2,dY2,dZ2,1,0,0);
		double mvmatrix2[16];
		glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix2);
		glPopMatrix();
		double inv_mvmatrix[16];
		Matrix44d temp(mvmatrix2);
		Invert(temp);
		for (int lauf=0; lauf<16; lauf++) inv_mvmatrix[lauf]=temp[lauf/4][lauf%4];

		double ttx,tty,ttz;
		calcCoord(dX,dY,0,mvmatrix,&ttx,&tty,&ttz);
		//mid=QPointF((double)mid.x()/(double)gla->curSiz.width()*2.0-1.0,(double)mid.y()/(double)gla->curSiz.height()*2.0-1.0);
		//mid=QPointF(mid.x()*2,mid.y()*2);
		//mid=QPointF(ttx,tty);
		//qDebug() <<mid<<" scale: "<<gla->trackball.track.sca<<"  "<<endl;
		/*double inv2_mvmatrix[16];
		Matrix44d temp2(projmatrix);
		Invert(temp2);
		for (int lauf=0; lauf<16; lauf++) inv2_mvmatrix[lauf]=temp2[lauf/4][lauf%4];
		calcCoord(mid.x(), mid.y(),0,inv2_mvmatrix,&ttx,&tty,&ttz);
		qDebug() << mid_temp << " "<< ttx<<" "<< tty << endl; */
	
		//calcCoord(0,pen.width.x()*paintbox->diag*0.01*0.5,0,mvmatrix,&tx,&ty,&tz);
		calcCoord(0,0,0,mvmatrix,&dX,&dY,&dZ);
		//radius=pen.width.x()*m.cm.bbox.Diag()*0.01*0.5;
		//radius=sqrt((ty-dY)*(ty-dY)+(tx-dX)*(tx-dX)+(tz-dZ)*(tz-dZ));
		calcCoord(0,1,0,mvmatrix,&tx,&ty,&tz);
		double scale_fac=sqrt((ty-dY)*(ty-dY)+(tx-dX)*(tx-dX)+(tz-dZ)*(tz-dZ));
		//qDebug() <<"scale_fac: "<< scale_fac << " radius: "<<radius<<endl;
		//for(fpi=temp.begin();fpi!=temp.end();++fpi) {
		//qDebug() << pen.radius << "  "<<scale_fac<<"  "<<old_size<<" diag: "<<m.cm.bbox.Diag()<<endl;
		for (int lauf2=0; lauf2<temp_po.size(); lauf2++) {
			CFaceO * fac=temp_po.at(lauf2);
			bool intern=false;

			double distance[3];

			for (int lauf=0; lauf<3; lauf++) { 
				//TODO problema: se la proiezione cambia c'e' un errore => melgio ricalcolare mid e mid_prev con mvmatrix ???
				double dx,dy,dz; // distance
				calcCoord((fac)->V(lauf)->P()[0],(fac)->V(lauf)->P()[1],(fac)->V(lauf)->P()[2],mvmatrix,&dx,&dy,&dz);
				gluProject((fac)->V(lauf)->P()[0],(fac)->V(lauf)->P()[1],(fac)->V(lauf)->P()[2],mvmatrix,projmatrix,viewport,&tx,&ty,&tz);
				//p[lauf]=QPointF(dx,dy);
				p[lauf]=QPointF(tx,ty);
				if (tx>=0 && tx<viewport[2] && ty>=0 && ty<viewport[3])
					z[lauf]=QPointF(tz,(GLfloat)pixels[(int)(((int)ty)*viewport[2]+(int)tx)]);
				else z[lauf]=QPoint(1,0);
				distance[lauf]=dz;
				//qDebug() << dx << " " << dy << " " << dz<<" "<<mid<<endl;
				/*double a,b,c;
				calcCoord((fac)->V(lauf)->P()[0],(fac)->V(lauf)->P()[1],(fac)->V(lauf)->P()[2],mvmatrix2,&a,&b,&c);
				gluProject((fac)->V(lauf)->P()[0],(fac)->V(lauf)->P()[1],(fac)->V(lauf)->P()[2],mvmatrix,projmatrix,viewport,&tx,&ty,&tz);
				if (tx>=0 && tx<old_size.x() && ty>=0 && ty<old_size.y())
					z[lauf]=QPointF(tz,(GLfloat)pixels[(int)(((int)ty)*old_size.x()+(int)tx)]);
				else z[lauf]=QPoint(1,0);
				p[lauf]=QPointF(a,b);*/
				//gluProject((fac)->V(lauf)->P()[0],(fac)->V(lauf)->P()[1],(fac)->V(lauf)->P()[2],mvmatrix,projmatrix,viewport,&tx,&ty,&tz);
				//p[lauf]=QPointF(tx,ty);
			}
	
			if (backface || isFront(p[0],p[1],p[2])) {
				for (int lauf=0; lauf<3; lauf++)if (invisible || (z[lauf].x()<=z[lauf].y()+0.005)) {
					tx=p[lauf].x();
					ty=p[lauf].y();
					//QPointF poo(mid.x()*(distance[lauf])*-1.0,mid.y()*(distance[lauf])*-1.0);
					//if (isIn(poo,poo,tx,ty,/*0.08*gla->trackball.track.sca*/radius)==1) {
					/** i have NO idea why it works with viewport[3]*0.88 */
					if (isIn(mid,mid_prev,tx,ty,pen.radius*scale_fac*viewport[3]*0.88/distance[lauf])==1) {
					//if (isIn(QPoint(0,0),QPoint(0,0),tx,ty,pen.radius)==1) {  
						intern=true;
						if (!sel_vert.contains(fac->V(lauf))) {
							risult->push_back(fac->V(lauf));
							sel_vert.insert(fac->V(lauf),fac->V(lauf));
							//qDebug() << tx << " " << ty << " " << tz <<"   "<< mid<<endl;
						}
					}
				}
				if ((pointInTriangle(mid,p[0],p[1],p[2]) || pointInTriangle(mid_prev,p[0],p[1],p[2]))) {
					intern=true;
				}
			}
			if (intern && ! selezionati.contains(fac)) {
				selezionati.insert((fac),(fac));
				actual->push_back(fac);
				for (int lauf=0; lauf<3; lauf++) {
					CFaceO * nf=(fac)->FFp(lauf);
					//qDebug () << "nf" << nf->V(0)->P()[0] << endl;
					if (!selezionati.contains(nf)) {
						//actual->push_back(nf);
						temp_po.push_back(nf);
					}
				}
			}
		}
	}
	//qDebug() << "----------"<< endl;
}

void EditPaintPlugin::fillFrom(MeshModel & m,CFaceO * face) {
	//qDebug() << "fillFrom" << endl;
	QHash <CFaceO *,CFaceO *> visited;
	QHash <CVertexO *,CVertexO *> temporaneo;
	vector <CFaceO *>temp_po;
	bool who=face->IsS();
	temp_po.push_back(face);
	visited.insert(face,face);
	int opac=paintbox->getOpacity();
	Color4b newcol=paintbox->getColor(curr_mouse);
	for (int lauf2=0; lauf2<temp_po.size(); lauf2++) {
		CFaceO * fac=temp_po.at(lauf2);
		if (who==fac->IsS()) {
			for (int lauf=0; lauf<3; lauf++) {
				if (!temporaneo.contains(fac->V(lauf))) {
					temporaneo.insert(fac->V(lauf),fac->V(lauf));
					colorize(fac->V(lauf),newcol,opac);
				}
			}
			for (int lauf=0; lauf<3; lauf++) { 
				if (!visited.contains(fac->FFp(lauf))) {
					temp_po.push_back(fac->FFp(lauf));
					visited.insert(fac->FFp(lauf),fac->FFp(lauf));
				}
			}
		}
	}
}

bool EditPaintPlugin::getFaceAtMouse(MeshModel &m,CMeshO::FacePointer& val) {
	QPoint mid=QPoint(cur.x(),inverse_y);
	return (GLPickTri<CMeshO>::PickNearestFace(mid.x(), mid.y(), m.cm, val,2,2));
}
bool EditPaintPlugin::getFacesAtMouse(MeshModel &m,vector<CMeshO::FacePointer> & val) {
	val.clear();
	QPoint mid=QPoint(cur.x(),inverse_y);
	GLPickTri<CMeshO>::PickFace(mid.x(), mid.y(), m.cm, val,2,2);
	return (val.size()>0);
}

bool EditPaintPlugin::getVertexAtMouse(MeshModel &m,CMeshO::VertexPointer& value) {
	CFaceO * temp=0;
	QPoint mid=QPoint(cur.x(),inverse_y);
	//qDebug() << "getVert" << mid << endl;
	//if (getFacesAtMouse(m,tempSel)) {
	double tx,ty,tz;
	if (getFaceAtMouse(m,temp)) {
		
		QPointF point[3];
		for (int lauf=0; lauf<3; lauf++) {
			gluProject(temp->V(lauf)->P()[0],temp->V(lauf)->P()[1],temp->V(lauf)->P()[2],mvmatrix,projmatrix,viewport,&tx,&ty,&tz);
			point[lauf]=QPointF(tx,ty);
		}
		value=temp->V(getNearest(mid,point,3));
	//qDebug() << "getVert2 " <<(int)value <<  endl;
		return true;
	}
	return false;
}

bool EditPaintPlugin::getVertexesAtMouse() {
	return false;
}

/** only in decorare it is possible to obtain the correct zbuffer values and other opengl stuff */
void EditPaintPlugin::Decorate(QAction * ac, MeshModel &m, GLArea * gla) {
	//qDebug()<<"startdecor"<<endl;
	//if (cur.x()==0 && cur.y()==0) return;
	updateMatrixes();
	QPoint mid=QPoint(cur.x(),gla->curSiz.height()-  cur.y());
	if (first) {
		//worker->waitTillPause();
		first=false;
		if (pixels!=0) { free(pixels); }
		pixels=(GLfloat *)malloc(sizeof(GLfloat)*gla->curSiz.width()*gla->curSiz.height());
		glReadPixels(0,0,gla->curSiz.width(),gla->curSiz.height(),GL_DEPTH_COMPONENT,GL_FLOAT,pixels);
		//worker->clear(pixels);
		//worker->setModelArea(&m,gla);
	}
	if(isDragging)
	{
	//qDebug() << "decorate" << endl;
	switch (paintbox->paintUtensil()) {
		case FILL: { 
			if (!pressed) return;
			CFaceO * temp_face;
			if(getFaceAtMouse(m,temp_face)) {
				fillFrom(m,temp_face);
			}
			pressed=false;
			return; 
		}
		case PICK: { 
			if (!pressed) return;
			CVertexO * temp_vert=0;
			if (paintbox->getPickMode()==0) {
				if (getVertexAtMouse(m,temp_vert)) {	
					//qDebug() << temp_vert->C()[0] << " " << temp_vert->C()[1] << endl;
					paintbox->setColor(temp_vert->C(),curr_mouse);
					
				} 
			} else {
				GLubyte pixel[3];
				glReadPixels( mid.x(),mid.y(), 1, 1,GL_RGB,GL_UNSIGNED_BYTE,(void *)pixel);
				paintbox->setColor(pixel[0],pixel[1],pixel[2],curr_mouse);
			}
			pressed=false;
			return; 
		}
		case PEN: {}
	}

	DrawXORRect(m,gla,false);

	/*PaintData pa;pa.start=cur;pa.end=prev;pa.pen=pen;
	pa.color=paintbox->getColor(curr_mouse);
	pa.opac=paintbox->getOpacity();
	worker->addData(pa);*/

	vector<CMeshO::VertexPointer>::iterator vpo;
	vector<CMeshO::VertexPointer> newSel;
	
	if (paintbox->searchMode()==2) curSel.clear();
	//GLPickTri<CMeshO>::PickFace(mid.x(), mid.y(), m.cm, curSel, pen.width.x(), pen.width.y());
	//CFaceO * tmp=0;
	//if (GLPickTri<CMeshO>::PickNearestFace(mid.x(), mid.y(), m.cm, tmp, pen.width.x(), pen.width.y()))

	getInternFaces(m,&curSel,&newSel,gla,pen,cur,prev,pixels,mvmatrix,projmatrix,viewport);

	int opac=paintbox->getOpacity();
	Color4b newcol=paintbox->getColor(curr_mouse);
	for(vpo=newSel.begin();vpo!=newSel.end();++vpo) {
		if (!temporaneo.contains(*vpo)) {
			temporaneo.insert(*vpo,(*vpo)->C());
			colorize(*vpo,newcol,opac);
		}
	}
	isDragging=false;
	pressed=false;
	}
	//qDebug()<<"enddecor"<<endl;
}

//void EditPaintPlugin::updateMe() {}

PaintWorker::PaintWorker() {
	nothingTodo=true;
	gla=0;
	mesh=0;
}

void PaintWorker::run() {
	int lauf=0;
	//exec();
	while(true) {
		PaintData data;
		mutex.lock();//------------------
		if (dati.size()==0) {
			nothingTodo=true;
			if (gla!=0) gla->update();
			//qDebug() << "worker-wait"<<endl;
			condition.wait(&mutex);
			//qDebug() << "worker-wait-end"<<endl;
			nothingTodo=false;
		} 
		data=dati[0];
		dati.pop_front();
		//qDebug() << "worker-wait-end2"<<endl;
		mutex.unlock();//-----------------
		//qDebug() << data.start<< " "<<data.end <<lauf << endl;

		vector<CMeshO::VertexPointer>::iterator vpo;
		vector<CMeshO::VertexPointer> newSel;
		//qDebug() << "worker-wait-end3"<<endl;
		//if (curSel.size()==0) {
			//getFacesAtMouse(m,curSel);
			//GLPickTri<CMeshO>::PickFace(data.start.x(),gla->curSiz.height()-  data.start.y(), mesh->cm, curSel, data.pen.radius, data.pen.radius);
			//CFaceO * tmp=0;
			/*CMeshO::FaceIterator fi;
        		for(fi=mesh->cm.face.begin();fi!=mesh->cm.face.end();++fi) 
            		if(!(*fi).IsD()) {
				curSel.push_back((&*fi));
			}*/
			curSel.clear();
			
		//}
		//qDebug() << "worker-wait-end4"<<endl;
		QPoint old_size=QPoint(gla->curSiz.width(),gla->curSiz.height());
		getInternFaces(*mesh,&curSel,&newSel,gla,data.pen,data.start,data.end,pixels,mvmatrix,projmatrix,viewport);
	
		for(vpo=newSel.begin();vpo!=newSel.end();++vpo) {
			if (!temporaneo.contains(*vpo)) {
				temporaneo.insert(*vpo,(*vpo)->C());
				colorize(*vpo,data.color,data.opac);
			}
		}
		if (gla!=0) gla->update();
	}
}

Q_EXPORT_PLUGIN(EditPaintPlugin)
