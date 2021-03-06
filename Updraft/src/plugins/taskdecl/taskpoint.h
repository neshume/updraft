#ifndef UPDRAFT_SRC_PLUGINS_TASKDECL_TASKPOINT_H_
#define UPDRAFT_SRC_PLUGINS_TASKDECL_TASKPOINT_H_

#include <QDomElement>
#include "assignedarea.h"
#include "util/location.h"

namespace Updraft {

struct TurnPoint;

/// Turning point in a task.
class TaskPoint {
 public:
  TaskPoint() {}

  /// Creates TaskPoint from DOM element.
  explicit TaskPoint(const QDomElement &elem);

  /// Stores TaskPoint to DOM element.
  /// \param doc pointer to DOM element representing document.
  /// \param taskPointElem pointer to DOM element representing TaskPoint.
  void toDom(QDomDocument *doc, QDomElement *taskPointElem) const;

  /// \return Code of TaskPoint or empty string if it doesn't exist
  QString getCode() const;

  /// \return Name of TaskPoint or empty string if it doesn't exist
  QString getName() const;

  /// \return Geographical location of the task point
  const Util::Location& getLocation() const;

  /// \return AssignedArea of the task point
  const AssignedArea& getAssignedArea() const;

  /// Gathers data from a TurnPoint and store them in the TaskPoint.
  void setTP(const TurnPoint *tp);

  /// Sets the new location. Deletes information about assigned TurnPoint.
  void setLocation(const Util::Location &location_);

  /// Sets the new name for this task point.
  void setName(const QString& name_);

  /// Sets shape of the assigned area.
  void setAssignedArea(const AssignedArea &assignedArea_);

 private:
  /// Code identifying the assigned TurnPoint.
  QString code;

  /// Name of the assigned TurnPoint.
  QString name;

  /// Geographical location of the task point
  Util::Location location;

  /// structure defining shape of the assigned area.
  AssignedArea area;
};

}  // End namespace Updraft

#endif  // UPDRAFT_SRC_PLUGINS_TASKDECL_TASKPOINT_H_
