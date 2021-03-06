#include "openairspace.h"

#include <QDebug>
#include <QFile>
#include <QString>


namespace OpenAirspace {
  Parser::Parser(const QString& fileName) {
    // qDebug("Parser ctor");
    this->allAirspaces = NULL;
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly)) {
      qDebug("OpenAirspace file not found : ");
      return;
    }

    QTextStream ts(&file);
    QString text("");

    while (text != "AC" && !ts.atEnd())
      ts >> text;
    // ts.seek(ts.pos() -2);
    bool acOn = true;

    if (ts.atEnd())
      qDebug("Not supported OpenAirspace format.");

    this->allAirspaces = new QVector<Airspace*>();
    while (!ts.atEnd()) {
      Airspace* nextairspace = new Airspace(&ts, &acOn);
      this->allAirspaces->push_back(nextairspace);
    }

    file.close();
  }
  Parser::~Parser(void) {
    // qDebug("Parser dtor");
    if (this->allAirspaces) {
      for (int i = 0; i < allAirspaces->size(); ++i) {
        Airspace* a = allAirspaces->at(i);
        delete a;
      }
      delete allAirspaces;
      allAirspaces = NULL;
    }
  }
}  // OpenAirspace
