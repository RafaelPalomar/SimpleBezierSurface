#include "vtkBezierSurfaceSource.h"

#include <rapidjson/document.h>

#include <vtkNew.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkMaskPoints.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPLYReader.h>
#include <vtkCenterOfMass.h>

#include <cstdlib>
#include <iostream>
#include <array>
#include <fstream>
#include <sstream>


//==============================================================================
//Constants
//==============================================================================
const unsigned int uResH = 300;
const unsigned int vResH = 300;

struct XYZ
{
  float x,y,z;
};

int main(int argc, char* argv[])
{

  // Argument checking
  if (argc!=3)
    {
    std::cerr << "Wrong number of arguments." << std::endl;
    return EXIT_FAILURE;
    }

  std::array<XYZ, 16> controlPoints = {0};

  // Load a file
 std::fstream file(argv[1]);
 std::string line;
 std::stringstream buffer;
 while(getline(file,line))
   {
   buffer << line;
   }

  rapidjson::Document document;
  document.Parse(buffer.str().c_str());

  // Extract the control points
  const rapidjson::Value &a = document["ControlPointPositions"];
  assert(a.IsArray());

  unsigned int i=0;

  for (auto& rows : a.GetArray())
    {
    for (auto& columns: rows.GetArray())
      {
      controlPoints[i].x = columns["x"].GetFloat();
      controlPoints[i].y = columns["y"].GetFloat();
      controlPoints[i].z = columns["z"].GetFloat();

      std::cout << "x: " << columns["x"].GetFloat() << std::endl;
      std::cout << "y: " << columns["y"].GetFloat() << std::endl;
      std::cout << "z: " << columns["z"].GetFloat() << std::endl;
      std::cout << "--" << std::endl;

      i++;

      }
      std::cout << "//" << std::endl;
    }

  // Extract the transformations
  const rapidjson::Value &r= document["Rotation"];
  const rapidjson::Value &t= document["Position"];
  const rapidjson::Value &s= document["Scale"];

  vtkNew<vtkPoints> points;
  for (auto p : controlPoints) {
    points->InsertNextPoint(p.x, p.y, p.z);
    std::cout << p.x << p.y << p.z << std::endl;
   }

  vtkNew<vtkPLYReader> modelReader;
  modelReader->SetFileName(argv[2]);

  vtkNew<vtkCenterOfMass> centerOfMass;
  centerOfMass->SetInputConnection(modelReader->GetOutputPort());
  centerOfMass->Update();
  double* center = centerOfMass->GetCenter();

  std::cout << *centerOfMass << std::endl;

  // Process control points
  vtkNew<vtkPolyData> controlPointsPoly;
  controlPointsPoly->SetPoints(points.GetPointer());

  vtkNew<vtkMaskPoints> maskPoints;
  maskPoints->SetInputData(controlPointsPoly.GetPointer());
  maskPoints->SetOnRatio(1);
  maskPoints->GenerateVerticesOn();
  maskPoints->SingleVertexPerCellOn();

  // Process bezier surface
  vtkNew<vtkBezierSurfaceSource> surface;
  surface->SetNumberOfControlPoints(4, 4);
  surface->SetResolution(10, 10);
  surface->SetControlPoints(points.GetPointer());
  surface->Update();

  // Create the transformation matrix
  vtkNew<vtkTransform> transform;
  transform->RotateX(r["x"].GetFloat());
  transform->RotateY(r["y"].GetFloat());
  transform->RotateZ(r["z"].GetFloat());
  transform->Scale(s["x"].GetFloat()*1000, s["y"].GetFloat()*1000, s["z"].GetFloat()*1000);
  transform->Translate(t["x"].GetFloat()-center[0]*1000,
                       t["y"].GetFloat()-center[1]*1000,
                       t["z"].GetFloat()-center[2]*1000);

  std::cout << *transform << std::endl;

  vtkNew<vtkTransformPolyDataFilter> bezierTransformFilter;
  bezierTransformFilter->SetTransform(transform.GetPointer());
  bezierTransformFilter->SetInputData(surface->GetOutput());
  bezierTransformFilter->Update();

  // Write out the bezier surface
  vtkNew<vtkXMLPolyDataWriter> bezierWriter;
  bezierWriter->SetFileName("/tmp/a.vtp");
  bezierWriter->SetInputData(surface->GetOutput());
  bezierWriter->Write();

  vtkNew<vtkTransformPolyDataFilter> controlTransformFilter;
  controlTransformFilter->SetTransform(transform.GetPointer());
  controlTransformFilter->SetInputConnection(maskPoints->GetOutputPort());

  // Write out the control points
  vtkNew<vtkXMLPolyDataWriter> controlWriter;
  controlWriter->SetFileName("/tmp/b.vtp");
  controlWriter->SetInputConnection(controlTransformFilter->GetOutputPort());
  controlWriter->Write();

  return EXIT_SUCCESS;
}
