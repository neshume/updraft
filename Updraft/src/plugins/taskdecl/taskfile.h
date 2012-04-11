#ifndef UPDRAFT_SRC_PLUGINS_TASKDECL_TASKFILE_H_
#define UPDRAFT_SRC_PLUGINS_TASKDECL_TASKFILE_H_

#include <QObject>
#include <QString>
#include "datahistory.h"

namespace osg {
  class EllipsoidModel;
}

namespace Updraft {

class TaskFile : public QObject {
  Q_OBJECT

 public:
  /// State of task storage (on disk)
  enum StorageState {
    UNSTORED_EMPTY,
    UNSTORED_EDITED,
    STORED_SYNCHRONIZED,
    STORED_DIRTY
  };

  /// Creates a new empty TaskFile.
  /// \param ellipsoid Ellipsoid model for distance calculations.
  explicit TaskFile(const osg::EllipsoidModel* ellipsoid);

  /// Creates a new TaskFile from specified file on a disk.
  /// If the file couldn't be loaded, empty task is created.
  /// \param filePath_ full path to a disk file
  /// \param ellipsoid Ellipsoid model for distance calculations.
  TaskFile(const QString &filePath_, const osg::EllipsoidModel* ellipsoid);

  /// \return File name
  QString getFileName() const;

  /// \return Absolute (full) file path
  QString getFilePath() const;

  /// \return State of file storage
  StorageState getStorageState() const;

  /// Save to current location.
  /// To obtain used path call getFilePath().
  void save();

  /// Save to new location.
  /// \param filePath_ new file path. Next call of save()
  /// will use this path.
  void saveAs(const QString &filePath_);

  /// Initialize editing session of current data.
  /// \return Current TaskData
  /// If other editing session is started, it returns NULL.
  TaskData* beginEdit();

  /// Ends session and optionally stores current state to history.
  /// \param storeState If it is true, current state is saved to history.
  void endEdit(bool storeState);

  /// Initializes reading session for current data.
  /// \return Current TaskData
  const TaskData* beginRead() const;

  /// Ends reading session.
  void endRead() const;

  /// Steps back in file history.
  void undo();

  /// Steps forward in file history.
  void redo();

 signals:
  /// Signal which is emitted when the content of the file has been changed.
  void dataChanged();

  /// Signal which is emitted when the storage state has been changed.
  void storageStateChanged();

 private:
  /// Proceeds update actions on either undo or redo.
  void updateOnUndoRedo();

  /// Full path to file in filesystem. Could be empty, if task is not saved.
  QString filePath;

  /// StorageState indicates whether file is saved on disk
  /// and if it has been changed since last save/load operation.
  StorageState storageState;

  /// DataHistory holds task editing history and also current data.
  DataHistory dataHistory;

  /// Flag indicating reading/editing process.
  /// Set in beginEdit(),beginRead(), freed in endEdit(),endRead().
  mutable bool locked;
};

}  // End namespace Updraft

#endif  // UPDRAFT_SRC_PLUGINS_TASKDECL_TASKFILE_H_
