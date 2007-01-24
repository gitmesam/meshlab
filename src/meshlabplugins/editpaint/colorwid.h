#ifndef COLORWID_H
#define COLORWID_H

class ColorWid : public QWidget {
protected:
	QColor bg;
public:
	ColorWid ( QWidget * parent = 0, Qt::WindowFlags f = 0 ) :QWidget(parent,f) {}
	void setColor(QColor co);
	inline QColor getColor() { return bg; }
	virtual void paintEvent ( QPaintEvent * event );
	virtual void mousePressEvent ( QMouseEvent * event );
};

#endif