/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vqDrawBoxQueryDialog.h"
#include "ui_drawBoxQuery.h"

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProgressBar>
#include <QResizeEvent>
#include <QSettings>
#include <QShowEvent>
#include <QVBoxLayout>

#include <qtStatusManager.h>
#include <qtStlUtil.h>
#include <qtUtil.h>

#include <vgFileDialog.h>

#include <vvMakeId.h>

#include "vqSettings.h"

QTE_IMPLEMENT_D_FUNC(vqDrawBoxQueryDialog)

//-----------------------------------------------------------------------------
// Custom graphics view that handles mouse events for box drawing
class DrawBoxGraphicsView : public QGraphicsView
{
public:
  DrawBoxGraphicsView(vqDrawBoxQueryDialog* dialog, QWidget* parent = 0) :
    QGraphicsView(parent),
    Dialog(dialog),
    Drawing(false),
    CurrentRect(0),
    ImageItem(0),
    NeedInitialFit(false)
    {}

  void setDrawingEnabled(bool enabled) { DrawingEnabled = enabled; }
  bool isDrawingEnabled() const { return DrawingEnabled; }

  QRectF lastDrawnRect() const { return LastDrawnRect; }
  bool hasLastDrawnRect() const { return HasLastDrawnRect; }
  void clearLastDrawnRect() { HasLastDrawnRect = false; }

  void setImageItem(QGraphicsPixmapItem* item)
    {
    ImageItem = item;
    // Request initial fit when a new image is set
    NeedInitialFit = (item != 0);
    }

  void fitToImage()
    {
    if (ImageItem && scene())
      {
      this->fitInView(ImageItem, Qt::KeepAspectRatio);
      }
    }

protected:
  void showEvent(QShowEvent* event) override
    {
    QGraphicsView::showEvent(event);
    // Only auto-fit on initial show when we have a pending fit request
    if (NeedInitialFit)
      {
      this->fitToImage();
      NeedInitialFit = false;
      }
    }

  void mousePressEvent(QMouseEvent* event) override
    {
    if (DrawingEnabled && event->button() == Qt::LeftButton)
      {
      Drawing = true;
      StartPoint = mapToScene(event->pos());
      CurrentRect = scene()->addRect(QRectF(StartPoint, StartPoint),
                                     QPen(Qt::red, 2),
                                     QBrush(QColor(255, 0, 0, 50)));
      }
    else
      {
      QGraphicsView::mousePressEvent(event);
      }
    }

  void mouseMoveEvent(QMouseEvent* event) override
    {
    if (Drawing && CurrentRect)
      {
      QPointF currentPoint = mapToScene(event->pos());
      QRectF rect = QRectF(StartPoint, currentPoint).normalized();
      CurrentRect->setRect(rect);
      }
    else
      {
      QGraphicsView::mouseMoveEvent(event);
      }
    }

  void mouseReleaseEvent(QMouseEvent* event) override
    {
    if (Drawing && event->button() == Qt::LeftButton)
      {
      Drawing = false;
      if (CurrentRect)
        {
        LastDrawnRect = CurrentRect->rect();
        HasLastDrawnRect = true;
        // Remove the temporary rectangle - it will be re-added as a permanent one
        scene()->removeItem(CurrentRect);
        delete CurrentRect;
        CurrentRect = 0;
        // Call the dialog's finalize method directly
        if (Dialog)
          {
          Dialog->finalizeBoxEdit();
          }
        }
      }
    else
      {
      QGraphicsView::mouseReleaseEvent(event);
      }
    }

private:
  vqDrawBoxQueryDialog* Dialog;
  bool DrawingEnabled = false;
  bool Drawing;
  bool HasLastDrawnRect = false;
  bool NeedInitialFit;
  QPointF StartPoint;
  QGraphicsRectItem* CurrentRect;
  QRectF LastDrawnRect;
  QGraphicsPixmapItem* ImageItem;
};

//-----------------------------------------------------------------------------
class vqDrawBoxQueryDialogPrivate
{
public:
  vqDrawBoxQueryDialogPrivate(vqDrawBoxQueryDialog* q) :
    q_ptr(q),
    NextBoxId(0),
    ImageHeight(0),
    ImageWidth(0)
    {}

  Ui::DrawBoxQueryDialog UI;
  QGraphicsScene* Scene;
  DrawBoxGraphicsView* GraphicsView;
  QGraphicsPixmapItem* ImageItem;

  QUrl ExemplarUri;

  struct DrawnBox
  {
    int Id;
    QRectF Region;
    QGraphicsRectItem* RectItem;
  };

  QList<DrawnBox> DrawnBoxes;
  int NextBoxId;
  int ImageHeight;
  int ImageWidth;

  qtStatusManager StatusManager;
  qtStatusSource StatusSource;

protected:
  QTE_DECLARE_PUBLIC_PTR(vqDrawBoxQueryDialog)

private:
  QTE_DECLARE_PUBLIC(vqDrawBoxQueryDialog)
};

///////////////////////////////////////////////////////////////////////////////

//BEGIN vqDrawBoxQueryDialog

//-----------------------------------------------------------------------------
vqDrawBoxQueryDialog::vqDrawBoxQueryDialog(
  QWidget* parent, Qt::WindowFlags flags)
  : QDialog(parent, flags), d_ptr(new vqDrawBoxQueryDialogPrivate(this))
{
  QTE_D(vqDrawBoxQueryDialog);

  d->UI.setupUi(this);

  // Create the graphics scene and view for image display
  d->Scene = new QGraphicsScene(this);
  d->GraphicsView = new DrawBoxGraphicsView(this, this);
  d->GraphicsView->setScene(d->Scene);
  d->GraphicsView->setRenderHint(QPainter::Antialiasing);
  d->GraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
  d->ImageItem = 0;

  QVBoxLayout* layout = new QVBoxLayout(d->UI.imagePlayerWidget);
  layout->addWidget(d->GraphicsView);
  layout->setMargin(0);

  qtUtil::setStandardIcons(d->UI.buttonBox);

  // Disable OK button until we have at least one box
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  // Connect UI controls
  connect(d->UI.chooseImage, SIGNAL(clicked()),
          this, SLOT(chooseImage()));
  connect(d->UI.addBox, SIGNAL(clicked()),
          this, SLOT(addBox()));
  connect(d->UI.removeBox, SIGNAL(clicked()),
          this, SLOT(removeSelectedBoxes()));
  connect(d->UI.clearBoxes, SIGNAL(clicked()),
          this, SLOT(clearAllBoxes()));
  connect(d->UI.boxList, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateButtonStates()));

  // Initial button states
  d->UI.addBox->setEnabled(false);
  d->UI.removeBox->setEnabled(false);
  d->UI.clearBoxes->setEnabled(false);

  // Restore geometry
  QSettings settings;
  settings.beginGroup("Window/DrawBoxQueryDialog");
  this->restoreGeometry(settings.value("geometry").toByteArray());
  d->UI.splitter->restoreState(settings.value("state").toByteArray());
}

//-----------------------------------------------------------------------------
vqDrawBoxQueryDialog::~vqDrawBoxQueryDialog()
{
  QTE_D(vqDrawBoxQueryDialog);

  // Save geometry
  QSettings settings;
  settings.beginGroup("Window/DrawBoxQueryDialog");
  settings.setValue("geometry", this->saveGeometry());
  settings.setValue("state", d->UI.splitter->saveState());
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::initialize()
{
  // No special initialization needed for QGraphicsView approach
}

//-----------------------------------------------------------------------------
std::string vqDrawBoxQueryDialog::exemplarUri() const
{
  QTE_D_CONST(vqDrawBoxQueryDialog);
  return stdString(d->ExemplarUri);
}

//-----------------------------------------------------------------------------
std::vector<vvImageBoundingBox> vqDrawBoxQueryDialog::drawnBoxes() const
{
  QTE_D_CONST(vqDrawBoxQueryDialog);

  std::vector<vvImageBoundingBox> boxes;
  boxes.reserve(d->DrawnBoxes.size());

  foreach (const vqDrawBoxQueryDialogPrivate::DrawnBox& db, d->DrawnBoxes)
    {
    vvImageBoundingBox box;
    // QGraphicsScene uses top-left origin, same as image coordinates
    box.TopLeft.X = static_cast<int>(db.Region.left());
    box.TopLeft.Y = static_cast<int>(db.Region.top());
    box.BottomRight.X = static_cast<int>(db.Region.right());
    box.BottomRight.Y = static_cast<int>(db.Region.bottom());
    boxes.push_back(box);
    }

  return boxes;
}

//-----------------------------------------------------------------------------
int vqDrawBoxQueryDialog::exec()
{
  QTE_D(vqDrawBoxQueryDialog);

  // If we don't have an image yet, ask for one
  if (d->ExemplarUri.isEmpty())
    {
    this->chooseImage();

    // Abort if user did not pick an image
    if (d->ExemplarUri.isEmpty())
      {
      this->reject();
      return QDialog::Rejected;
      }
    }

  return QDialog::exec();
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::chooseImage()
{
  QTE_D(vqDrawBoxQueryDialog);

  QString fileName = vgFileDialog::getOpenFileName(
    this, "Select Query Image...", QString(),
    "Image files (*.png *.jpg *.jpeg *.bmp *.tif *.tiff);;"
    "All files (*)");

  if (!fileName.isEmpty())
    {
    QPixmap pixmap(fileName);
    if (pixmap.isNull())
      {
      QMessageBox::warning(this, "Failed to load image",
        "Could not load the selected image file. Please check the path and "
        "try again.");
      return;
      }

    // Clear any existing content
    d->Scene->clear();
    d->DrawnBoxes.clear();
    d->UI.boxList->clear();
    d->ImageItem = 0;
    d->GraphicsView->setImageItem(0);

    // Set the URI
    d->ExemplarUri = QUrl::fromLocalFile(fileName);
    d->UI.imageLocation->setText(fileName);

    // Add the image to the scene
    d->ImageItem = d->Scene->addPixmap(pixmap);
    d->ImageHeight = pixmap.height();
    d->ImageWidth = pixmap.width();
    d->Scene->setSceneRect(pixmap.rect());

    // Tell the graphics view about the image item so it can auto-fit on resize
    d->GraphicsView->setImageItem(d->ImageItem);
    d->GraphicsView->fitToImage();

    // Enable box drawing
    d->UI.addBox->setEnabled(true);

    this->updateButtonStates();
    }
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::addBox()
{
  QTE_D(vqDrawBoxQueryDialog);

  if (d->ExemplarUri.isEmpty())
    {
    return;
    }

  // Enable drawing mode
  d->GraphicsView->setDrawingEnabled(true);
  d->GraphicsView->setDragMode(QGraphicsView::NoDrag);
  d->GraphicsView->setCursor(Qt::CrossCursor);
  d->UI.addBox->setEnabled(false);
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::finalizeBoxEdit()
{
  QTE_D(vqDrawBoxQueryDialog);

  if (!d->GraphicsView->hasLastDrawnRect())
    {
    this->cancelBoxEdit();
    return;
    }

  // Get the drawn region
  QRectF region = d->GraphicsView->lastDrawnRect();
  d->GraphicsView->clearLastDrawnRect();

  // Clamp to image bounds
  region = region.intersected(QRectF(0, 0, d->ImageWidth, d->ImageHeight));

  if (region.width() < 5 || region.height() < 5)
    {
    // Too small, ignore
    this->cancelBoxEdit();
    return;
    }

  // Add permanent rectangle to scene
  QGraphicsRectItem* rectItem = d->Scene->addRect(
    region, QPen(Qt::green, 2), QBrush(QColor(0, 255, 0, 50)));

  // Add to our list
  vqDrawBoxQueryDialogPrivate::DrawnBox box;
  box.Id = d->NextBoxId++;
  box.Region = region;
  box.RectItem = rectItem;
  d->DrawnBoxes.append(box);

  // Update UI
  this->updateBoxList();
  this->cancelBoxEdit();
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::cancelBoxEdit()
{
  QTE_D(vqDrawBoxQueryDialog);

  d->GraphicsView->setDrawingEnabled(false);
  d->GraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
  d->GraphicsView->unsetCursor();
  d->UI.addBox->setEnabled(true);
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::unsetBoxEditCursor()
{
  QTE_D(vqDrawBoxQueryDialog);
  d->GraphicsView->unsetCursor();
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::removeSelectedBoxes()
{
  QTE_D(vqDrawBoxQueryDialog);

  QList<QListWidgetItem*> selectedItems = d->UI.boxList->selectedItems();
  if (selectedItems.isEmpty())
    {
    return;
    }

  // Get IDs of selected boxes
  QSet<int> idsToRemove;
  foreach (QListWidgetItem* item, selectedItems)
    {
    idsToRemove.insert(item->data(Qt::UserRole).toInt());
    }

  // Remove matching boxes from our list and scene
  QMutableListIterator<vqDrawBoxQueryDialogPrivate::DrawnBox> iter(d->DrawnBoxes);
  while (iter.hasNext())
    {
    vqDrawBoxQueryDialogPrivate::DrawnBox& box = iter.next();
    if (idsToRemove.contains(box.Id))
      {
      if (box.RectItem)
        {
        d->Scene->removeItem(box.RectItem);
        delete box.RectItem;
        }
      iter.remove();
      }
    }

  this->updateBoxList();
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::clearAllBoxes()
{
  QTE_D(vqDrawBoxQueryDialog);

  // Remove all box rectangles from scene
  foreach (const vqDrawBoxQueryDialogPrivate::DrawnBox& box, d->DrawnBoxes)
    {
    if (box.RectItem)
      {
      d->Scene->removeItem(box.RectItem);
      delete box.RectItem;
      }
    }

  d->DrawnBoxes.clear();
  this->updateBoxList();
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::updateBoxList()
{
  QTE_D(vqDrawBoxQueryDialog);

  d->UI.boxList->clear();

  foreach (const vqDrawBoxQueryDialogPrivate::DrawnBox& box, d->DrawnBoxes)
    {
    QString text = QString("Box %1: (%2, %3) - (%4, %5)")
                     .arg(box.Id + 1)
                     .arg(static_cast<int>(box.Region.left()))
                     .arg(static_cast<int>(box.Region.top()))
                     .arg(static_cast<int>(box.Region.right()))
                     .arg(static_cast<int>(box.Region.bottom()));

    QListWidgetItem* item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, box.Id);
    d->UI.boxList->addItem(item);
    }

  this->updateButtonStates();
}

//-----------------------------------------------------------------------------
void vqDrawBoxQueryDialog::updateButtonStates()
{
  QTE_D(vqDrawBoxQueryDialog);

  bool haveBoxes = !d->DrawnBoxes.isEmpty();
  bool haveSelection = !d->UI.boxList->selectedItems().isEmpty();

  d->UI.removeBox->setEnabled(haveSelection);
  d->UI.clearBoxes->setEnabled(haveBoxes);
  d->UI.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(haveBoxes);
}

//END vqDrawBoxQueryDialog
