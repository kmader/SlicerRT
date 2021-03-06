/*==============================================================================

  Copyright (c) Radiation Medicine Program, University Health Network,
  Princess Margaret Hospital, Toronto, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kevin Wang, Princess Margaret Cancer Centre 
  and was supported by Cancer Care Ontario (CCO)'s ACRU program 
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).

==============================================================================*/

#ifndef __vtkMRMLRTBeamNode_h
#define __vtkMRMLRTBeamNode_h

// Beams includes
#include "vtkSlicerBeamsModuleMRMLExport.h"

// MRML includes
#include <vtkMRMLModelNode.h>

class vtkPolyData;
class vtkMRMLScene;
class vtkMRMLDoubleArrayNode;
class vtkMRMLRTPlanNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLSegmentationNode;

/// \ingroup SlicerRt_QtModules_Beams
class VTK_SLICER_BEAMS_MODULE_MRML_EXPORT vtkMRMLRTBeamNode : public vtkMRMLModelNode
{
public:
  static const char* NEW_BEAM_NODE_NAME_PREFIX;

  enum
  {
    /// Fired if beam geometry (beam model) needs to be updated
    BeamGeometryModified = 62200,
    /// Fired if beam transform needs to be updated
    BeamTransformModified,
    /// Invoke if the beam is to be cloned.
    /// External Beam Planning logic processes the event if exists
    CloningRequested
  };

public:
  static vtkMRMLRTBeamNode *New();
  vtkTypeMacro(vtkMRMLRTBeamNode,vtkMRMLModelNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Create instance of a GAD node. 
  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes from name/value pairs 
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format. 
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object 
  virtual void Copy(vtkMRMLNode *node);

  /// Make sure display node and transform node are present and valid
  virtual void SetScene(vtkMRMLScene* scene);

  /// Get unique node XML tag name (like Volume, Model) 
  virtual const char* GetNodeTagName() { return "RTBeam"; };

  /// Create and observe default display node
  virtual void CreateDefaultDisplayNodes();

  /// Create transform node that places the beam poly data in the right position based on geometry.
  /// Only creates it if missing
  virtual void CreateDefaultTransformNode();

  /// Create transform node that places the beam poly data in the right position based on geometry.
  /// Always creates a new transform node.
  virtual void CreateNewBeamTransformNode();

  /// Update beam transform based on beam and plan parameters
  void UpdateTransform();

  /// Update beam poly data based on beam geometry parameters (jaws, MLC)
  void UpdateGeometry();

  /// Invoke cloning requested event. External Beam Planning logic processes the event and
  /// clones the beam if exists
  void RequestCloning();

public:
  /// Get parent plan node
  vtkMRMLRTPlanNode* GetParentPlanNode();

  /// Get MLC position double array node
  vtkMRMLDoubleArrayNode* GetMLCPositionDoubleArrayNode();
  /// Set and observe MLC position double array node.
  /// Triggers \sa BeamGeometryModified event and re-generation of beam model
  void SetAndObserveMLCPositionDoubleArrayNode(vtkMRMLDoubleArrayNode* node);

  /// Get DRR volume node
  vtkMRMLScalarVolumeNode* GetDRRVolumeNode();
  /// Set and observe DRR volume node
  void SetAndObserveDRRVolumeNode(vtkMRMLScalarVolumeNode* node);

  /// Get contour BEV node
  vtkMRMLScalarVolumeNode* GetContourBEVVolumeNode();
  /// Set and observe contour BEV node
  void SetAndObserveContourBEVVolumeNode(vtkMRMLScalarVolumeNode* node);

  /// Get isocenter position from parent plan
  /// \return Success flag
  bool GetPlanIsocenterPosition(double isocenter[3]);

  /// Calculate source position using gantry angle, SAD, and isocenter
  /// \return Success flag
  bool CalculateSourcePosition(double source[3]);

// Beam parameters
public:
  /// Get beam number
  vtkGetMacro(BeamNumber, int);
  /// Set beam number
  vtkSetMacro(BeamNumber, int);

  /// Get beam description
  vtkGetStringMacro(BeamDescription);
  /// Set beam description
  vtkSetStringMacro(BeamDescription);

  /// Get X1 jaw position
  vtkGetMacro(X1Jaw, double);
  /// Set X1 jaw position. Triggers \sa BeamGeometryModified event and re-generation of beam model
  void SetX1Jaw(double x1Jaw);

  /// Get X2 jaw position
  vtkGetMacro(X2Jaw, double);
  /// Set X2 jaw position. Triggers \sa BeamGeometryModified event and re-generation of beam model
  void SetX2Jaw(double x2Jaw);

  /// Get Y1 jaw position
  vtkGetMacro(Y1Jaw, double);
  /// Set Y1 jaw position. Triggers \sa BeamGeometryModified event and re-generation of beam model
  void SetY1Jaw(double y1Jaw);

  /// Get Y2 jaw position
  vtkGetMacro(Y2Jaw, double);
  /// Set Y2 jaw position. Triggers \sa BeamGeometryModified event and re-generation of beam model
  void SetY2Jaw(double y2Jaw);

  /// Get source-axis distance
  vtkGetMacro(SAD, double);
  /// Set source-axis distance. Triggers \sa BeamGeometryModified event and re-generation of beam model
  void SetSAD(double sad);

  /// Get gantry angle
  vtkGetMacro(GantryAngle, double);
  /// Set gantry angle. Triggers \sa BeamTransformModified event and re-generation of beam model
  void SetGantryAngle(double angle);

  /// Get collimator angle
  vtkGetMacro(CollimatorAngle, double);
  /// Set collimator angle. Triggers \sa BeamTransformModified event and re-generation of beam model
  void SetCollimatorAngle(double angle);

  /// Get couch angle
  vtkGetMacro(CouchAngle, double);
  /// Set couch angle. Triggers \sa BeamTransformModified event and re-generation of beam model
  void SetCouchAngle(double angle);

  /// Get beam weight
  vtkGetMacro(BeamWeight, double);
  /// Set beam weight
  vtkSetMacro(BeamWeight, double);

protected:
  /// Create beam model from beam parameters, supporting MLC leaves
  void CreateBeamPolyData(vtkPolyData* beamModelPolyData);

protected:
  vtkMRMLRTBeamNode();
  ~vtkMRMLRTBeamNode();
  vtkMRMLRTBeamNode(const vtkMRMLRTBeamNode&);
  void operator=(const vtkMRMLRTBeamNode&);

// Beam properties
protected:
  /// Beam number
  int  BeamNumber;
  /// Beam description
  char* BeamDescription;
  /// Beam weight, taken into account when accumulating per-beam doses
  double BeamWeight;

  /// X1 jaw position
  double X1Jaw;
  /// X2 jaw position
  double X2Jaw;
  /// Y1 jaw position
  double Y1Jaw;
  /// Y2 jaw position
  double Y2Jaw;
  /// Source-axis distance
  double SAD;

  /// Gantry angle
  double GantryAngle;
  /// Collimator angle
  double CollimatorAngle;
  /// Couch angle
  double CouchAngle;
};

#endif // __vtkMRMLRTBeamNode_h
