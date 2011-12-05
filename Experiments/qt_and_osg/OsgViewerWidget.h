#ifndef OSGVIEWER_H
#define OSGVIEWER_H

#include <QWidget>
#include <QTimer>

#include <osgQt/GraphicsWindowQt>
#include <osgViewer/Viewer>

#include <osg/Node>
#include <osgDB/ReadFile>

class OsgViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OsgViewerWidget(QWidget *parent = 0);
    ~OsgViewerWidget();

    osgViewer::Viewer* viewer;
    osg::Camera* camera;
    osg::Group* root;

public slots:
    void checkUpdate();

protected:
    void paintEvent(QPaintEvent *);
    QTimer timer;
};

#endif // OSGVIEWER_H
