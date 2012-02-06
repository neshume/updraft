#include "openedfile.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>

#include <QDebug>
#include <QPushButton>

namespace Updraft {
namespace IgcViewer {

OpenedFile::~OpenedFile() {
  if (tab) {
    delete tab;
  }

  viewer->mapLayerGroup->removeMapLayer(track);

  viewer->opened.removeAll(this);
}

bool OpenedFile::init(IgcViewer* viewer, const QString& filename) {
  this->viewer = viewer;

  fileInfo = QFileInfo(filename);

  if (!igc.load(filename)) {
    qDebug() << "Loading IGC file failed.";
    return false;
  }

  createTab();
  createTrack();

  return true;
}

void OpenedFile::tabClosed() {
  tab = NULL;  // tab is already deleted
  delete this;
}

void OpenedFile::createTab() {
  QPushButton *b = new QPushButton("a nice graph should be here");

  tab = viewer->core->createTab(b, fileInfo.fileName());

  tab->connectSignalClosed(this, SLOT(tabClosed()));
}

void OpenedFile::createTrack() {
  const osg::EllipsoidModel* ellipsoid = viewer->core->getEllipsoidModel();

  osg::Geode* geode = new osg::Geode();

  osg::Geometry* geom = new osg::Geometry();
  geode->addDrawable(geom);

  osg::DrawArrays* drawArrayLines =
    new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP);
  geom->addPrimitiveSet(drawArrayLines);

  osg::Vec3Array* vertices = new osg::Vec3Array();
  geom->setVertexArray(vertices);

  osg::Vec4Array* colors = new osg::Vec4Array;
  geom->setColorArray(colors);
  geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  qreal minAltitude = 10000;
  qreal maxAltitude = -10000;

  foreach(Igc::Event const* ev, igc.events()) {
    if (ev->type != Igc::Event::FIX) {
      continue;
    }
    Igc::Fix const* fix = static_cast<Igc::Fix const*>(ev);

    if (minAltitude > fix->gpsLoc.alt) {
      minAltitude = fix->gpsLoc.alt;
    }

    if (maxAltitude < fix->gpsLoc.alt) {
      maxAltitude = fix->gpsLoc.alt;
    }
  }

  foreach(Igc::Event const* ev, igc.events()) {
    if (ev->type != Igc::Event::FIX) {
      continue;
    }
    Igc::Fix const* fix = static_cast<Igc::Fix const*>(ev);

    double x, y, z;

    ellipsoid->convertLatLongHeightToXYZ(
      fix->gpsLoc.lat_radians(), fix->gpsLoc.lon_radians(), fix->gpsLoc.alt,
      x, y, z);

    vertices->push_back(osg::Vec3(x, y, z));
    colors->push_back(osg::Vec4(
      0.5, 0.5, (fix->gpsLoc.alt - minAltitude) / (maxAltitude - minAltitude),
      1.0));
  }

  drawArrayLines->setFirst(0);
  drawArrayLines->setCount(vertices->size());

  osg::LineWidth* linewidth = new osg::LineWidth();
  linewidth->setWidth(2000.0f);

  geode->getOrCreateStateSet()->setAttributeAndModes(linewidth,
  osg::StateAttribute::ON);

  geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

  track = viewer->mapLayerGroup->insertMapLayer(geode, fileInfo.fileName());
}

}  // End namespace IgcViewer
}  // End namespace Updraft

