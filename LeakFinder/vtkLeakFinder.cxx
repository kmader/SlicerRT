/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

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

#include "vtkLeakFinder.h"

#include "StackWalker.h"

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkDebugLeaks.h"
#include "vtksys/SystemTools.hxx" 

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkLeakFinder);

//----------------------------------------------------------------------------
/*!
 * \ingroup SlicerRt_LeakFinder
 * \brief Class that extracts the call stack and saves it in a string.
 */
class StackWalkerStringOutput : public StackWalker
{
public:
  StackWalkerStringOutput()
  {
    m_LastStackTraceString = "";
  }

  /// Get last stack trace string (that is filled with calling ShowCallstack). Also clears the result string.
  std::string GetLastStackTraceString()
  {
    std::string lastStackTraceString = m_LastStackTraceString;
    m_LastStackTraceString = "";
    return lastStackTraceString;
  }

  /// Overridden function that appends the call stack element to the output string
  virtual void OnOutput(LPCSTR buffer)
  {
    m_LastStackTraceString.append(buffer);
  }

protected:
  /// Output string containing the call stack
  std::string m_LastStackTraceString;
};



//----------------------------------------------------------------------------
/*!
 * \ingroup SlicerRt_LeakFinder
 * \brief Debug leaks observer variant that keeps a record of the created objects and the call stacks at the point of creation
 */
class vtkLeakFinderObserver : public vtkDebugLeaksObserver
{
public:
  vtkLeakFinderObserver()
  {
    m_ObjectTraceEntries.clear();
    m_StackWalker = new StackWalkerStringOutput();
    m_OldDebugLeakObserver = NULL;

    std::string dateTime = vtksys::SystemTools::GetCurrentDateTime("%Y%m%d_%H%M%S");
    m_OutputFileName = std::string("./trace_") + dateTime + ".log";

    m_TraceRegisterAndUnregister = true;
  }

  virtual ~vtkLeakFinderObserver()
  {
    std::map<vtkObjectBase*, std::vector<std::string> >::iterator it;
    for (it = m_ObjectTraceEntries.begin(); it != m_ObjectTraceEntries.end(); ++it)
    {
      it->second.clear();
    }
    m_ObjectTraceEntries.clear();

    m_OldDebugLeakObserver = NULL;

    if (m_StackWalker)
    {
      delete m_StackWalker;
      m_StackWalker = NULL;
    }
  }

  /// Callback function that is called every time a VTK class is instantiated
  virtual void ConstructingObject(vtkObjectBase* o)
  {
    m_StackWalker->ShowCallstack();

    std::vector<std::string> stackTraceVector;
    std::string constructingStackTrace("----Construct stack trace----\n");
    constructingStackTrace.append(m_StackWalker->GetLastStackTraceString());
    stackTraceVector.push_back(constructingStackTrace);
    m_ObjectTraceEntries[o] = stackTraceVector;

    if (m_OldDebugLeakObserver)
    {
      m_OldDebugLeakObserver->ConstructingObject(o);
    }
  }

  /// Callback function that is called every time a VTK class is deleted
  virtual void DestructingObject(vtkObjectBase* o)
  {
    m_ObjectTraceEntries[o].clear();
    m_ObjectTraceEntries.erase(o);

    if (m_OldDebugLeakObserver)
    {
      m_OldDebugLeakObserver->DestructingObject(o);
    }
  }

  /// Callback function that is called at the last possible moment before the application exits.
  /// This function ends the tracing and writes the leak report to a file.
  /// Note: This callback function is only called when a patch is applied to VTK defining the signature of this
  ///  function in the vtkDebugLeaksObserver abstract class, and calling it in vtkDebugLeaks::ClassFinalize()
  virtual void Finalizing()
  {
    this->RestoreOldObserver();

    std::ofstream f;
    f.open(m_OutputFileName.c_str(), ios::trunc);
    f << this->GetLeakReport();
    f.close();
  }

  /// Callback function that is called upon registering an object
  /// Note: This callback function is only called when a patch is applied to VTK defining the signature of this
  ///  function in the vtkDebugLeaksObserver abstract class, and calling it in vtkDebugLeaks::ClassFinalize()
  virtual void RegisteringObject(vtkObjectBase* o)
  {
    if (m_TraceRegisterAndUnregister)
    {
      m_StackWalker->ShowCallstack();

      std::string registeringStackTrace("----Register stack trace-----\n");
      registeringStackTrace.append(m_StackWalker->GetLastStackTraceString());
      m_ObjectTraceEntries[o].push_back(registeringStackTrace);
    }

    if (m_OldDebugLeakObserver)
    {
      // Uncomment this if you have an other observer you want to use at the same time that supports Register/Unregister
      //m_OldDebugLeakObserver->RegisteringObject(o);
    }
  }

  /// Callback function that is called upon unregistering an object
  /// Note: This callback function is only called when a patch is applied to VTK defining the signature of this
  ///  function in the vtkDebugLeaksObserver abstract class, and calling it in vtkDebugLeaks::ClassFinalize()
  virtual void UnregisteringObject(vtkObjectBase* o)
  {
    if (m_TraceRegisterAndUnregister)
    {
      m_StackWalker->ShowCallstack();

      std::string unregisteringStackTrace("----Unregister stack trace---\n");
      unregisteringStackTrace.append(m_StackWalker->GetLastStackTraceString());
      m_ObjectTraceEntries[o].push_back(unregisteringStackTrace);
    }

    if (m_OldDebugLeakObserver)
    {
      // Uncomment this if you have an other observer you want to use at the same time that supports Register/Unregister
      //m_OldDebugLeakObserver->UnregisteringObject(o);
    }
  }

  /// Returns a string containing information (pointer, type and call stack at the point of creation)
  /// of objects that have been created but not deleted between registering and unregistering of this observer class
  std::string GetLeakReport()
  {
    if (vtkDebugLeaks::GetDebugLeaksObserver() != m_OldDebugLeakObserver)
    {
      return "Cannot get report while running. Observer needs to be disconnected (vtkLeakFinder::EndTracing called) first.";
    }

    std::string report("");
    std::map<vtkObjectBase*, std::vector<std::string> >::iterator entriesIt;
    for (entriesIt=m_ObjectTraceEntries.begin(); entriesIt!=m_ObjectTraceEntries.end(); ++entriesIt)
    {
      std::stringstream ss;
      ss.setf(ios::hex,ios::basefield);
      ss << std::endl << std::endl;
      ss << "Pointer: " << entriesIt->first << " (type: " << entriesIt->first->GetClassName() << ")" << std::endl;
      for (std::vector<std::string>::iterator it = entriesIt->second.begin(); it != entriesIt->second.end(); ++it)
      {
        ss << (*it) << std::endl;
      }
      ss << std::endl;
      report.append(ss.str());
    }
    return report;
  }

  /// Saves the previously set observer (its callbacks are also called)
  void SetOldDebugLeakObserver(vtkDebugLeaksObserver* oldObserver)
  {
    m_OldDebugLeakObserver = oldObserver;
  }

  /// Release this observer and restore the previous one that was saved
  void RestoreOldObserver() 
  {
    vtkDebugLeaks::SetDebugLeaksObserver(m_OldDebugLeakObserver);
    this->SetOldDebugLeakObserver(NULL);
  }

  /// Set output file name
  void SetOutputFileName(std::string fileName)
  {
    m_OutputFileName = fileName;
  }

  /// Set trace register/unregister flag
  void SetTraceRegisterAndUnregister(bool trace)
  {
    m_TraceRegisterAndUnregister = trace;
  }

protected:
  /// Stack walker object that extracts the call stack
  StackWalkerStringOutput* m_StackWalker;

  /// Map containing the constructed VTK objects and the call stacks
  ///   at the point of their creation, registers and unregisters
  std::map<vtkObjectBase*, std::vector<std::string> > m_ObjectTraceEntries;

  /// Previously set debug leaks observer
  vtkDebugLeaksObserver* m_OldDebugLeakObserver;

  /// Output file name that is used when saving the leak report in the Finalizing function
  std::string m_OutputFileName;

  /// Flag whether Register and Unregister calls are traced (their call stack saved)
  bool m_TraceRegisterAndUnregister;
};



//----------------------------------------------------------------------------
vtkLeakFinder::vtkLeakFinder()
{
  this->Observer = NULL;
  this->Observer = new vtkLeakFinderObserver();
}

//----------------------------------------------------------------------------
vtkLeakFinder::~vtkLeakFinder()
{
  if (this->Observer)
  {
    delete this->Observer;
    this->Observer = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkLeakFinder::StartTracing()
{
  if (vtkDebugLeaks::GetDebugLeaksObserver())
  {
    this->Observer->SetOldDebugLeakObserver(vtkDebugLeaks::GetDebugLeaksObserver());
  }

  vtkDebugLeaks::SetDebugLeaksObserver(this->Observer);
}

//----------------------------------------------------------------------------
void vtkLeakFinder::EndTracing()
{
  this->Observer->Finalizing();
}

//----------------------------------------------------------------------------
void vtkLeakFinder::SetOutputFileName(std::string fileName)
{
  this->Observer->SetOutputFileName(fileName);
}

//----------------------------------------------------------------------------
void vtkLeakFinder::SetTraceRegisterAndUnregister(bool trace)
{
  this->Observer->SetTraceRegisterAndUnregister(trace);
}
