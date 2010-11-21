#include <QHeaderView>
#include <QVBoxLayout>
#include <QDebug>
#include <QMap>

#include "openfilemodel.h"
#include "filelistitemdelegate.h"
#include "globaldispatcher.h"
#include "basefile.h"
#include "filelist.h"
#include "editor.h"

class AutoExpandTreeView : public QTreeView
{
public:
	virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) { this->expandAll(); }
};

FileList::FileList(QWidget *parent) :
    QDockWidget(parent)
{
	setWindowTitle("Open Files");

	mTreeView = new AutoExpandTreeView();
	mTreeView->setModel(&gOpenFileModel);
	mTreeView->setMinimumWidth(150);
	mTreeView->header()->hide();
	mTreeView->setItemDelegate(new FileListItemDelegate(mTreeView));
	mTreeView->setAttribute(Qt::WA_MacShowFocusRect, false);
	mTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
	mTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	mTreeView->viewport()->setAttribute(Qt::WA_Hover);
	mTreeView->header()->setStretchLastSection(false);
	mTreeView->header()->setResizeMode(0, QHeaderView::Stretch);
	mTreeView->header()->setResizeMode(1, QHeaderView::Fixed);
	mTreeView->header()->resizeSection(1, 16);

	setWidget(mTreeView);

	connect(gDispatcher, SIGNAL(selectFile(BaseFile*)), this, SLOT(selectFile(BaseFile*)));
	connect(mTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(fileSelected()));
	connect(mTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
}

BaseFile* FileList::getSelectedFile()
{
	QModelIndex selectedIndex = mTreeView->selectionModel()->currentIndex();
	if (selectedIndex.isValid())
		return (BaseFile*)gOpenFileModel.data(selectedIndex, OpenFileModel::FileRole).value<void*>();

	return NULL;
}

void FileList::selectFile(BaseFile* file)
{
	if (!file) return;
	if (getSelectedFile() != file)
	{
		QModelIndex index = gOpenFileModel.findFile(file);
		mTreeView->setCurrentIndex(index);
	}
}

void FileList::fileSelected()
{
	BaseFile* file = getSelectedFile();
	gDispatcher->emitSelectFile(file);
}

void FileList::itemClicked(QModelIndex index)
{
	if (index.column() == 1)
		gOpenFileModel.closeButtonClicked(index);
}












