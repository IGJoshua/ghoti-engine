ffi.cdef[[

void kmRay2Fill(kmRay2* ray, kmScalar px, kmScalar py, kmScalar vx,
                kmScalar vy);
void kmRay2FillWithEndpoints( kmRay2 *ray, const kmVec2 *start,
                              const kmVec2 *end );

/*
    Lines are defined by a pt and a vector. It outputs the vector
    multiply factor that gives the intersection point
*/
kmBool kmLine2WithLineIntersection(const kmVec2 *ptA, const kmVec2 *vecA,
                                   const kmVec2 *ptB, const kmVec2 *vecB,
                                   kmScalar *outTA, kmScalar *outTB,
                                   kmVec2 *outIntersection );

kmBool kmSegment2WithSegmentIntersection( const kmRay2 *segmentA,
                                          const kmRay2 *segmentB,
                                          kmVec2 *intersection );

kmBool kmRay2IntersectLineSegment(const kmRay2* ray, const kmVec2* p1,
                                  const kmVec2* p2, kmVec2* intersection);
kmBool kmRay2IntersectTriangle(const kmRay2* ray, const kmVec2* p1,
                               const kmVec2* p2, const kmVec2* p3,
                               kmVec2* intersection, kmVec2* normal_out,
                               kmScalar* distance);

kmBool kmRay2IntersectBox(const kmRay2* ray, const kmVec2* p1,
                          const kmVec2* p2, const kmVec2* p3,
                          const kmVec2* p4, kmVec2* intersection,
                          kmVec2* normal_out);

kmBool kmRay2IntersectCircle(const kmRay2* ray, const kmVec2 centre,
                             const kmScalar radius, kmVec2* intersection);


]]
