#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FDumperv2.h"

#include "ctpl_stl.hpp"
#include "CBaseDumper.h"

#include <qstringlistmodel.h>

class FAQueueModel;
class FAReportModel;


class FDumperv2 : public QMainWindow
{
	Q_OBJECT
public:
	explicit FDumperv2(QWidget *parent = Q_NULLPTR);

	static std::shared_ptr<FDumperv2> getInstance() {
		if (!m_instance.operator std::shared_ptr<FDumperv2>()) 
			m_instance.operator=(std::make_shared<FDumperv2>());
		return m_instance;	
	}

private slots:
	virtual void OnAddQueueButton();

private:
	std::unique_ptr<FAQueueModel> m_queueModel;
	std::unique_ptr<FAReportModel> m_reportModel;
	
	static ThreadLock<std::shared_ptr<FDumperv2>> m_instance;
	ctpl::thread_pool m_pool;
	Ui::FDumperv2Class m_ui;
	ConfigMgr m_config;
};


class FAQueueModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	FAQueueModel(QObject* parent = Q_NULLPTR, int columbs = 1) : QAbstractTableModel(parent) {}
	void insertData(const int& column, const int& part, const QString& data) {
		m_data[column][part] = data;
	}

	int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE {
		return m_data.size();
	}
	int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE {
		return 4;
	}
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE {
		if (!index.isValid() || role != Qt::DisplayRole) {
			return QVariant();
		}
		if (index.column() == 0) {
			return m_data.at(index.row());
		}
		else if (index.column() == 1) {
			return m_data.at(index.row());
		}
		else if (index.column() == 2) {
			return m_data.at(index.row());
		}
		else if (index.column() == 3) {
			return m_data.at(index.row());
		}
		return QVariant();
	}
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE {
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
			if (section == 0) {
				return "Username";
			}
			else if (section == 1) {
				return "Status";
			}
			else if (section == 2) {
				return "Images";
			}
			else if (section == 3) {
				return "Progress";
			}
		}
		return QVariant();
	}
private:
	std::vector<QList<QString>> m_data;
};

class FAReportModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	FAReportModel(QObject* parent = Q_NULLPTR, int columbs = 1) : QAbstractTableModel(parent) {}

	void insertData(const int& column, const int& part, const QString& data) {
		m_data[column][part] = data;
	}

	int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE {
		return m_data.size();
	}
	int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE {
		return 4;
	}
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE {
		if (!index.isValid() || role != Qt::DisplayRole) {
			return QVariant();
		}
		if (index.column() == 0) {
			return m_data.at(index.row());
		}
		else if (index.column() == 1) {
			return m_data.at(index.row());
		}
		else if (index.column() == 2) {
			return m_data.at(index.row());
		}
		else if (index.column() == 3) {
			return m_data.at(index.row());
		}
		return QVariant();
	}
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE {
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
			if (section == 0) {
				return "Title";
			}
			else if (section == 1) {
				return "Date Posted";
			}
			else if (section == 2) {
				return "Resolution";
			}
			else if (section == 3) {
				return "ID";
			}
		}
		return QVariant();
	}

private:
	std::vector<QList<QString>> m_data;
};