// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vtkVgHull2D.h"

#include <vtkCellArray.h>
#include <vtkMath.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkType.h>

vtkStandardNewMacro(vtkVgHull2D);

// Construct an the hull object with no lines
vtkVgHull2D::vtkVgHull2D()
{
  this->Lines             = NULL;
  this->LinesStorageSize  = 0;
  this->NumberOfLines     = 0;
}

// Destructor for a hull object - remove the lines if necessary
vtkVgHull2D::~vtkVgHull2D()
{
  if (this->Lines)
    {
    delete [] this->Lines;
    this->Lines = NULL;
    }
}

// Remove all lines.
void vtkVgHull2D::RemoveAllLines()
{
  if (this->Lines)
    {
    delete [] this->Lines;
    this->Lines = NULL;
    }

  this->LinesStorageSize  = 0;
  this->NumberOfLines     = 0;
  this->Modified();
}

// Add a line. The vector (A,B) is the line "normal" and is from the
// line equation Ax + By + C = 0. The normal should point outwards
// away from the center of the hull.
int vtkVgHull2D::AddLine(double A, double B)
{
  double*     tmpPointer;
  int       i;
  double     norm, dotproduct;

  // Normalize the direction,
  // and make sure the vector has a length.
  norm = sqrt(A * A + B * B);
  if (norm == 0.0)
    {
    vtkErrorMacro(<< "Zero length vector not allowed for line normal!");
    return -VTK_INT_MAX;
    }
  A /= norm;
  B /= norm;

  // Check that it is at least somewhat different from the other
  // lines we have so far - can't have a normalized dot product of
  // nearly 1.
  for (i = 0; i < this->NumberOfLines; i++)
    {
    dotproduct =
      A * this->Lines[i * 3 + 0] +
      B * this->Lines[i * 3 + 1];

    //If lines are parallel, we already have the line.
    //Indicate this with the appropriate return value.
    if (dotproduct > 0.99999 && dotproduct < 1.00001)
      {
      return -(i + 1);
      }
    }

  // If adding this line would put us over the amount of space we've
  // allocated for lines, then we'll have to allocated some more space
  if ((this->NumberOfLines + 1) >= this->LinesStorageSize)
    {
    // Hang onto the previous set of lines
    tmpPointer = this->Lines;

    // Increase our storage
    if (this->LinesStorageSize <= 0)
      {
      this->LinesStorageSize = 100;
      }
    else
      {
      this->LinesStorageSize *= 2;
      }
    this->Lines = new double [this->LinesStorageSize * 3];

    if (!this->Lines)
      {
      vtkErrorMacro(<< "Unable to allocate space for lines");
      this->Lines = tmpPointer;
      return -VTK_INT_MAX;
      }

    // Copy the lines and delete the old storage space
    for (i = 0; i < this->NumberOfLines * 3; i++)
      {
      this->Lines[i] = tmpPointer[i];
      }
    if (tmpPointer)
      {
      delete [] tmpPointer;
      }
    }

  // Add the line at the end of the array.
  // The fourth element doesn't actually need to be set, but it
  // eliminates a purify uninitialized memory copy error if it is set
  i = this->NumberOfLines;
  this->Lines[i * 3 + 0] = A;
  this->Lines[i * 3 + 1] = B;
  this->Lines[i * 3 + 2] = 0.0;
  this->NumberOfLines++;

  this->Modified();

  // Return the index to this line so that it can be set later
  return i;
}

// Add a line, passing the line normal vector as a double array instead
// of three doubles.
int vtkVgHull2D::AddLine(double line[2])
{
  return this->AddLine(line[0], line[1], line[2]);
}

// Set a specific line - this line should already have been added with
// AddLine, and the return value then used to modifiy the line normal
// with this method.
void vtkVgHull2D::SetLine(int i, double A, double B)
{
  double norm;

  // Make sure this is a line that was already added
  if (i < 0 || i >= this->NumberOfLines)
    {
    vtkErrorMacro(<< "Invalid index in SetLine");
    return;
    }

  double* line = this->Lines + i * 4;
  if (A == line[0] && B == line[1])
    {
    return; //no modified
    }

  // Set line that has index i. Normalize the direction,
  // and make sure the vector has a length.
  norm = sqrt(A * A + B * B);
  if (norm == 0.0)
    {
    vtkErrorMacro(<< "Zero length vector not allowed for line normal!");
    return;
    }
  this->Lines[i * 3 + 0] = A / norm;
  this->Lines[i * 3 + 1] = B / norm;

  this->Modified();
}

// Set a specific line (that has already been added) - passing the line
// normal as a double array
void vtkVgHull2D::SetLine(int i, double line[2])
{
  this->SetLine(i, line[0], line[1], line[2]);
}

int vtkVgHull2D::AddLine(double A, double B, double C)
{
  int i, j;

  if ((i = this->AddLine(A, B)) >= 0)
    {
    this->Lines[3 * i + 2] = C;
    }
  else if (i >= -this->NumberOfLines)
    {
    //pick the D that minimizes the convex set
    j = -i - 1;
    this->Lines[3 * j + 2] = (C > this->Lines[3 * j + 2] ?
                              C : this->Lines[3 * j + 2]);
    }
  return i;
}

int vtkVgHull2D::AddLine(double line[2], double C)
{
  int i, j;

  if ((i = this->AddLine(line[0], line[1], line[2])) >= 0)
    {
    this->Lines[3 * i + 2] = C;
    }
  else if (i >= -this->NumberOfLines)
    {
    //pick the C that minimizes the convex set
    j = -i - 1;
    this->Lines[3 * j + 2] = (C > this->Lines[3 * j + 2] ?
                              C : this->Lines[3 * j + 2]);
    }
  return i;
}

void vtkVgHull2D::SetLine(int i, double A, double B, double C)
{
  if (i >= 0 && i < this->NumberOfLines)
    {
    double* line = this->Lines + 4 * i;
    if (line[0] != A || line[1] != B || line[2] != C)
      {
      this->SetLine(i, A, B);
      line[3] = C;
      this->Modified();
      }
    }
}

void vtkVgHull2D::SetLine(int i, double line[2], double C)
{
  this->SetLine(i, line[0], line[1], C);
}

// Add the four lines that represent the edges on a square
void vtkVgHull2D::AddSquareEdgeLines()
{
  this->AddLine(1.0,  0.0);
  this->AddLine(-1.0,  0.0);
  this->AddLine(0.0,  1.0);
  this->AddLine(0.0, -1.0);
}

// Add the four lines that represent the vertices on a square - halfway
// between the two adjacent edge lines.
void vtkVgHull2D::AddSquareVertexLines()
{
  this->AddLine(1.0,  1.0);
  this->AddLine(1.0, -1.0);
  this->AddLine(-1.0,  1.0);
  this->AddLine(-1.0, -1.0);
}

// Add the lines that represent the normals of the vertices of a
// circle formed by recursively subdividing the edges in a
// square.  The level indicates how many subdivisions to do with a
// level of 0 used to add the 4 lines from the original square, level 1
// will add 8 lines, and so on.
void vtkVgHull2D::AddRecursiveCircleLines(int level)
{
  if (level < 0)
    {
    vtkErrorMacro(<< "Cannot have a level less than 0!");
    return;
    }

  if (level > 20)
    {
    vtkErrorMacro(<< "Cannot have a level greater than 20!");
    return;
    }

  int numEdges = 4 * (1 << level);
  double* points = new double[2 * numEdges];
  int* edges = new int[2 * numEdges];

  // add initial points
  int idx = 0;
  points[idx++] =  1.0; points[idx++] =  0.0;
  points[idx++] =  0.0; points[idx++] =  1.0;
  points[idx++] = -1.0; points[idx++] =  0.0;
  points[idx++] =  0.0; points[idx++] = -1.0;
  int pointCount = 4;

  // add initial edges
  idx = 0;
  edges[idx++] = 0; edges[idx++] = 1;
  edges[idx++] = 1; edges[idx++] = 2;
  edges[idx++] = 2; edges[idx++] = 3;
  edges[idx++] = 3; edges[idx++] = 0;
  int edgeCount = 4;

  for (int i = 0; i < level; ++i)
    {
    int limit = edgeCount;
    for (int j = 0; j < limit; ++j)
      {
      int A = edges[j * 2 + 0];
      int B = edges[j * 2 + 1];

      // add new point to the end of the list
      points[pointCount * 2 + 0] = (points[2 * A + 0] + points[2 * B + 0]) * 0.5;
      points[pointCount * 2 + 1] = (points[2 * A + 1] + points[2 * B + 1]) * 0.5;

      // add first new edge in place of old edge
      edges[j * 2 + 0] = A;
      edges[j * 2 + 1] = pointCount;

      // add second new edge at end of list
      edges[edgeCount * 2 + 0] = pointCount;
      edges[edgeCount * 2 + 1] = B;

      ++pointCount;
      ++edgeCount;
      }
    }

  for (int i = 0; i < pointCount; ++i)
    {
    this->AddLine(points[i * 2 + 0], points[i * 2 + 1]);
    }

  delete [] points;
  delete [] edges;
}

// Create the n-sided convex hull from the input geometry according to the
// set of lines.
int vtkVgHull2D::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData* input = vtkPolyData::SafeDownCast(
                         inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(
                          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // There should be at least three points for this to work.
  if (input->GetNumberOfPoints() < 2)
    {
    vtkErrorMacro(<< "There must be>= 2 points in the input data!!!\n");
    return 1;
    }

  // There should be at least three lines for this to work. There will need
  // to be more lines than three if any of them are parallel.
  if (this->NumberOfLines < 3)
    {
    vtkErrorMacro(<< "There must be>= 3 lines!!!\n");
    return 1;
    }

  // Create a new set of points and lines into which the results will
  // be stored
  vtkPoints* outPoints = vtkPoints::New();
  vtkCellArray* outLines  = vtkCellArray::New();

  // Compute the D value for each line according to the vertices in the
  // geometry
  this->ComputeLineDistances(input);
  this->UpdateProgress(0.25);

  // Create a large segment representing each line, and clip that segment
  // against all other lines to form the edges of the hull.
  this->ClipSegmentsFromLines(outPoints, outLines, input->GetBounds());
  this->UpdateProgress(0.80);

  // Set the output vertices and lines
  output->SetPoints(outPoints);
  output->SetLines(outLines);

  // Delete the temporary storage
  outPoints->Delete();
  outLines->Delete();

  return 1;
}

// Compute the D value for each line. This is the largest D value obtained
// by passing a line with the specified normal through each vertex in the
// geometry. This line will have a normal pointing in towards the center of
// the hull.
void vtkVgHull2D::ComputeLineDistances(vtkPolyData* input)
{
  vtkIdType      i;
  int            j;
  double         coord[3];
  double         v;

  // Initialize all lines to the first vertex value
  input->GetPoint(0, coord);
  for (j = 0; j < this->NumberOfLines; j++)
    {
    this->Lines[j * 3 + 2] = -(this->Lines[j * 3 + 0] * coord[0] +
                               this->Lines[j * 3 + 1] * coord[1]);
    }
  // For all other vertices in the geometry, check if it produces a larger
  // D value for each of the lines.
  for (i = 1; i < input->GetNumberOfPoints(); i++)
    {
    input->GetPoint(i, coord);
    for (j = 0; j < this->NumberOfLines; j++)
      {
      v =  -(this->Lines[j * 3 + 0] * coord[0] +
             this->Lines[j * 3 + 1] * coord[1]);
      // negative means further in + direction of line
      if (v < this->Lines[j * 3 + 2])
        {
        this->Lines[j * 3 + 2] = v;
        }
      }
    }
}

// Given the set of lines, create a large segment for each, then use all the
// other lines to clip this polygon.
void vtkVgHull2D::ClipSegmentsFromLines(vtkPoints* outPoints,
                                        vtkCellArray* outLines,
                                        double* bounds)
{
  double verts[2 * 3];

  // create a segment for each line
  for (int i = 0; i < this->NumberOfLines; ++i)
    {
    this->CreateInitialSegment(verts, i, bounds);
    bool bothVertsClipped = false;

    // clip this segment against all other lines
    for (int j = 0; j < this->NumberOfLines; ++j)
      {
      if (i == j)
        {
        continue;
        }

      double dist1 = vtkMath::Dot2D(&verts[0 * 3 + 0], &this->Lines[j * 3]) +
                     this->Lines[j * 3 + 2];

      double dist2 = vtkMath::Dot2D(&verts[1 * 3 + 0], &this->Lines[j * 3]) +
                     this->Lines[j * 3 + 2];

      bool clipped1 = dist1 > 0.0;
      bool clipped2 = dist2 > 0.0;

      // this line clips the segment if the two verts are on opposite sides
      if ((clipped1) != (clipped2))
        {
        int vertToMove = clipped1 ? 0 : 1;
        double crossing = -dist1 / (dist2 - dist1);

        verts[vertToMove * 3 + 0] =
          verts[0] + crossing * (verts[1 * 3 + 0] - verts[0]);
        verts[vertToMove * 3 + 1] =
          verts[1] + crossing * (verts[1 * 3 + 1] - verts[1]);
        }
      else if (clipped2)
        {
        // both verts are on the outside of the line - throw out this segment
        bothVertsClipped = true;
        break;
        }
      }

    // add line segment to the output if we have one left
    if (!bothVertsClipped)
      {
      vtkIdType pnts[2];
      pnts[0] = outPoints->InsertNextPoint(verts + 0);
      pnts[1] = outPoints->InsertNextPoint(verts + 3);
      outLines->InsertNextCell(2, pnts);
      }
    }
}

// Create a sufficiently large segment from a line that will to cover the
// bounds of the input
void vtkVgHull2D::CreateInitialSegment(double* verts, int line, double* bounds)
{
  double center[3];

  center[0] = (bounds[0] + bounds[1]) * 0.5;
  center[1] = (bounds[2] + bounds[3]) * 0.5;
  center[2] = (bounds[4] + bounds[5]) * 0.5;

  double d =
    this->Lines[line * 3 + 0] * center[0] +
    this->Lines[line * 3 + 1] * center[1] +
    this->Lines[line * 3 + 2];

  // project bounding box center onto this line
  double lineCenter[2];
  lineCenter[0] = center[0] - d * this->Lines[line * 3 + 0];
  lineCenter[1] = center[1] - d * this->Lines[line * 3 + 1];

  double lineVec[2];
  lineVec[0] = -this->Lines[line * 3 + 1];
  lineVec[1] = this->Lines[line * 3];

  // construct a segment large enough to cover the XY bounding rect
  d =
    (bounds[1] - bounds[0]) +
    (bounds[3] - bounds[2]);

  verts[0 * 3 + 0] = lineCenter[0] + d * lineVec[0];
  verts[0 * 3 + 1] = lineCenter[1] + d * lineVec[1];
  verts[0 * 3 + 2] = center[2];
  verts[1 * 3 + 0] = lineCenter[0] - d * lineVec[0];
  verts[1 * 3 + 1] = lineCenter[1] - d * lineVec[1];
  verts[1 * 3 + 2] = center[2];
}

void vtkVgHull2D::GenerateHull(vtkPolyData* pd, double xmin, double xmax,
                               double ymin, double ymax, double zmin, double zmax)
{
  double bounds[6];
  bounds[0] = xmin; bounds[1] = xmax;
  bounds[2] = ymin; bounds[3] = ymax;
  bounds[4] = zmin; bounds[5] = zmax;

  this->GenerateHull(pd, bounds);
}

void vtkVgHull2D::GenerateHull(vtkPolyData* pd, double* bounds)
{
  // There should be at least three lines for this to work. There will need
  // to be more lines than three if any of them are parallel.
  if (this->NumberOfLines < 3)
    {
    vtkErrorMacro(<< "There must be>= 3 lines!!!");
    return;
    }

  // Create a new set of points and lines into which the results will
  // be stored
  vtkPoints* newPoints = vtkPoints::New();
  newPoints->Allocate(this->NumberOfLines * 3);

  vtkCellArray* newLines  = vtkCellArray::New();
  newLines->Allocate(newLines->EstimateSize(this->NumberOfLines, 3));

  this->ClipSegmentsFromLines(newPoints, newLines, bounds);

  pd->SetPoints(newPoints);
  pd->SetLines(newLines);
  newPoints->Delete();
  newLines->Delete();

  pd->Squeeze();
}

// Print the object
void vtkVgHull2D::PrintSelf(ostream& os, vtkIndent indent)
{
  int i;

  this->Superclass::PrintSelf(os, indent);

  os << indent << "Number Of Lines: " << this->NumberOfLines << endl;

  for (i = 0; i < this->NumberOfLines; i++)
    {
    os << indent << "Line " << i << ":  "
       << this->Lines[i * 4] << " "
       << this->Lines[i * 4 + 1] << " "
       << this->Lines[i * 4 + 2] << endl;
    }
}
