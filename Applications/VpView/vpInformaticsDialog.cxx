/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "ui_qtInformaticsDialog.h"

#include "vpInformaticsDialog.h"
#include "QVTKWidget.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTexture.h>
#include <vtkPNGReader.h>
#include <vtkGraph.h>
#include <vtkRandomGraphSource.h>
#include <vtkIntArray.h>
#include <vtkStringArray.h>
#include <vtkGraphLayoutView.h>
#include <vtkRenderedGraphRepresentation.h>
#include <vtkDataSetAttributes.h>

#include <vtksys/SystemTools.hxx>

namespace
{
enum enumGraphLayouts
{
  SpanTree,
  ForceDirected,
  Circular,
  Clustering2D,
  NumGraphLayouts
};

const char* LayoutLabels[NumGraphLayouts] =
{
  "Span Tree",
  "Force Directed",
  "Circular",
  "Clustering 2D"
};
}

//-----------------------------------------------------------------------------
vpInformaticsDialog::vpInformaticsDialog(QWidget* parent, Qt::WindowFlags flags) :
  QDialog(parent, flags)
{
  this->InternalWidget = new Ui::qtInformaticsDialog;
  this->InternalWidget->setupUi(this);

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Graph = 0x0;

  this->GraphView = vtkSmartPointer<vtkGraphLayoutView>::New();

  this->IconsFile = std::string("");

  for (int i = 0; i < NumGraphLayouts; ++i)
    {
    this->InternalWidget->layoutCombo->addItem(tr(LayoutLabels[i]));
    }

  this->InternalWidget->iconSizeSlider->setMinimum(5);
  this->InternalWidget->iconSizeSlider->setMaximum(50);
  this->InternalWidget->iconSizeSlider->setValue(24);

  connect(this->InternalWidget->layoutCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onLayoutChanged(int)));
  connect(this->InternalWidget->iconSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(onIconSizeChanged(int)));
}

//-----------------------------------------------------------------------------
vpInformaticsDialog::~vpInformaticsDialog()
{
  delete this->InternalWidget;
}

//-----------------------------------------------------------------------------
vtkRenderWindow* vpInformaticsDialog::GetRenderWindow()
{
  return this->InternalWidget->qvtkRenderWidget->GetRenderWindow();
}

void vpInformaticsDialog::SetRenderWindow(vtkRenderWindow* renWin)
{
  this->InternalWidget->qvtkRenderWidget->SetRenderWindow(renWin);
}

QVTKWidget* vpInformaticsDialog::GetWidget()
{
  return this->InternalWidget->qvtkRenderWidget;
}

//-----------------------------------------------------------------------------
void vpInformaticsDialog::Initialize()
{
  if (this->IconsFile.empty())
    {
    std::cerr << "ERROR: Missing icons image file. " << std::endl;
    return;
    }

  int numNodes = 120;
  vtkSmartPointer<vtkRandomGraphSource> random = vtkSmartPointer<vtkRandomGraphSource>::New();
  random->SetNumberOfVertices(numNodes);
  random->StartWithTreeOn();
  random->Update();

  this->Graph = random->GetOutput();
  vtkSmartPointer<vtkIntArray> index = vtkSmartPointer<vtkIntArray>::New();
  index->SetName("IconIndex");
  vtkSmartPointer<vtkStringArray> name = vtkSmartPointer<vtkStringArray>::New();
  name->SetName("Name");

  int cacheIdx = 1;
  int iedIdx = 12;
  int terrIdx = 5;
  int docIdx = 14;
  int houseIdx = 9;

  std::string label;
  int numIcons = 5;
  for (int i = 0; i < numNodes; ++i)
    {
    switch ((i % numIcons))
      {
      case 0:
        label = "Cache " + i / numIcons;
        name->InsertNextValue(label.c_str());
        index->InsertNextValue(cacheIdx);
        break;
      case 1:
        label = "IED " + i / numIcons;
        name->InsertNextValue(label.c_str());
        index->InsertNextValue(iedIdx);
        break;
      case 2:
        label = "person " + i / numIcons;
        name->InsertNextValue(label.c_str());
        index->InsertNextValue(terrIdx);
        break;
      case 3:
        label = "document " + i / numIcons;
        name->InsertNextValue(label.c_str());
        index->InsertNextValue(docIdx);
        break;
      case 4:
        label = "place " + i / numIcons;
        name->InsertNextValue(label.c_str());
        index->InsertNextValue(houseIdx);
        break;
      default:
        cout << "Bad icon index\n";
      }
    }

  this->Graph->GetVertexData()->AddArray(index);
  this->Graph->GetVertexData()->AddArray(name);

  vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
  reader->SetFileName(IconsFile.c_str());

  vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
  texture->SetInputConnection(reader->GetOutputPort());

  this->GraphView = vtkSmartPointer<vtkGraphLayoutView>::New();
  this->GraphView->SetIconTexture(texture);
  this->GraphView->SetIconSize(64, 64);
  this->updateIconSize();
  this->updateLayout();

  this->GraphView->SetInteractor(this->GetWidget()->GetInteractor());
  this->SetRenderWindow(this->GraphView->GetRenderWindow());

  // In C++ need to downcast rep to vtkRenderedGraphRepresentation
  vtkRenderedGraphRepresentation* rep =
    dynamic_cast<vtkRenderedGraphRepresentation*>(this->GraphView->SetRepresentationFromInput(Graph));
  rep->UseVertexIconTypeMapOff();
  rep->SetVertexSelectedIcon(12);
  rep->SetVertexIconSelectionModeToSelectedIcon();
  rep->VertexIconVisibilityOn();
  rep->SetVertexIconArrayName("IconIndex");
  //rep.VertexLabelVisibilityOn()
  //rep.SetVertexLabelArrayName('Name')

  //view->GetRenderWindow()->SetSize(500, 500);
  this->GraphView->ResetCamera();
  this->GraphView->GetInteractor()->Initialize();
}

void vpInformaticsDialog::updateLayout()
{
  switch (this->InternalWidget->layoutCombo->currentIndex())
    {
    case SpanTree:
      this->GraphView->SetLayoutStrategyToSpanTree();
      break;

    case ForceDirected:
      this->GraphView->SetLayoutStrategyToForceDirected();
      break;

    case Circular:
      this->GraphView->SetLayoutStrategyToCircular();
      break;

    case Clustering2D:
      this->GraphView->SetLayoutStrategyToClustering2D();
      break;
    }
}

void vpInformaticsDialog::updateIconSize()
{
  int size = this->InternalWidget->iconSizeSlider->value();
  this->GraphView->SetDisplaySize(size, size);
}

void vpInformaticsDialog::onLayoutChanged(int /*index*/)
{
  this->updateLayout();
  this->GraphView->ResetCamera();
  this->GraphView->Render();
}

void vpInformaticsDialog::onIconSizeChanged(int /*size*/)
{
  this->updateIconSize();
  this->GraphView->Render();
}
