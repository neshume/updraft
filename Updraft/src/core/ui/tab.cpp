#include "tab.h"

#include <QTabWidget>
#include <QHBoxLayout>

namespace Updraft {
namespace Core {

/// Add a new tab to parent.
/// Takes ownership of content.
Tab::Tab(QWidget* content, QString title, QTabWidget* parent)
  : widget(content), tabWidget(parent) {
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->addWidget(widget);
  widget->setParent(this);

  int index = tabWidget->addTab(this, title);
  tabWidget->setCurrentIndex(index);
}

Tab::~Tab() {
}

void Tab::select() {
  int id = tabWidget->indexOf(this);
  tabWidget->setCurrentIndex(id);
}

void Tab::setTitle(const QString &title) {
  tabWidget->setTabText(tabWidget->indexOf(this), title);
}

/// Close this tab.
/// This method deletes the tab and its widget.
void Tab::close() {
  int id = tabWidget->indexOf(this);
  tabWidget->removeTab(id);
  emit tabRemoved(this);

  delete this;
}

void Tab::connectSlotClose(const QObject* sender, const char *signal) {
  connect(sender, signal, this, SLOT(close()));
}

void Tab::connectSignalCloseRequested(const QObject* receiver,
  const char *method) {
  connect(this, SIGNAL(tabCloseRequested()), receiver, method);
}

void Tab::connectCloseRequestToClose() {
  connect(this, SIGNAL(tabCloseRequested()), this, SLOT(close()));
}

void Tab::connectSignalSelected(const QObject* receiver, const char *method) {
  connect(this, SIGNAL(selected()), receiver, method);
}

void Tab::connectSignalDeselected(const QObject* receiver, const char *method) {
  connect(this, SIGNAL(deselected()), receiver, method);
}

}  // End namespace Core
}  // End namespace Updraft

