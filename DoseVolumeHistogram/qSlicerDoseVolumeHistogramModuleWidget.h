/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerDoseVolumeHistogramModuleWidget_h
#define __qSlicerDoseVolumeHistogramModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerDoseVolumeHistogramModuleExport.h"

// STD includes
#include <map>

class qSlicerDoseVolumeHistogramModuleWidgetPrivate;
class vtkMRMLNode;
class QCheckBox;
class QLineEdit;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_DOSEVOLUMEHISTOGRAM_EXPORT qSlicerDoseVolumeHistogramModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerDoseVolumeHistogramModuleWidget(QWidget *parent=0);
  virtual ~qSlicerDoseVolumeHistogramModuleWidget();

public slots:

protected slots:
  void doseVolumeNodeChanged(vtkMRMLNode*);
  void structureSetNodeChanged(vtkMRMLNode*);
  void chartNodeChanged(vtkMRMLNode*);

  void computeDvhClicked();
  void showInChartCheckStateChanged(int aState);
  void exportDvhToCsvClicked();
  void exportMetricsToCsv();
  void showHideAllCheckedStateChanged(int aState);
  void showMetricsCheckedStateChanged(int aState);
  void lineEditMetricEdited(QString aText);

  void onLogicModified();

protected:
  QScopedPointer<qSlicerDoseVolumeHistogramModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

protected:
  /// Updates button states
  void updateButtonsState();

  /// Updates state of show/hide chart checkboxes according to the currently selected chart
  void updateChartCheckboxesState();

  /// Refresh DVH statistics table
  void refreshDvhTable();

  /// Get value list from text in given line edit (empty list if unsuccessful)
  void getNumbersFromLineEdit(QLineEdit* aLineEdit, std::vector<double> &aValues);

protected:
  /// Map that associates a string pair containing the structure set plot name (including table row number) and the vtkMRMLDoubleArrayNode id (respectively) to the show/hide in chart checkboxes
  std::map<QCheckBox*, std::pair<std::string, std::string>> m_ChartCheckboxToStructureSetNameMap;

  /// Vector of checkbox states for the case the user makes the show/hide all checkbox state partially checked. Then the last configuration is restored
  std::vector<bool> m_ShowInChartCheckStates;

  /// Flag whether show/hide all checkbox has been clicked - some operations are not necessary when it was clicked
  bool m_ShowHideAllClicked;

private:
  Q_DECLARE_PRIVATE(qSlicerDoseVolumeHistogramModuleWidget);
  Q_DISABLE_COPY(qSlicerDoseVolumeHistogramModuleWidget);
};

#endif