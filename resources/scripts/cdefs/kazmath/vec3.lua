ffi.cdef[[

kmVec3* kmVec3Zero(kmVec3* pOut);
kmVec3* kmVec3Fill(kmVec3* pOut, kmScalar x, kmScalar y, kmScalar z);
kmVec3* kmVec3Scale(kmVec3* pOut, const kmVec3* pIn, const kmScalar s);
kmVec3* kmVec3Assign(kmVec3* pOut, const kmVec3* pIn);
kmBool kmVec3AreEqual(const kmVec3* p1, const kmVec3* p2);

kmVec3* kmVec3Add(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);
kmVec3* kmVec3Subtract(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);
kmVec3* kmVec3Mul( kmVec3* pOut,const kmVec3* pV1, const kmVec3* pV2 );
kmVec3* kmVec3Div( kmVec3* pOut,const kmVec3* pV1, const kmVec3* pV2 );

kmScalar kmVec3Length(const kmVec3* pIn);
kmScalar kmVec3LengthSq(const kmVec3* pIn);

kmVec3* kmVec3Lerp(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2,
  kmScalar t);

kmVec3* kmVec3Normalize(kmVec3* pOut, const kmVec3* pIn);

kmScalar kmVec3Dot(const kmVec3* pV1, const kmVec3* pV2);
kmVec3* kmVec3Cross(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);

kmVec3* kmVec3MultiplyMat3(kmVec3 *pOut, const kmVec3 *pV,
  const kmMat3* pM);
kmVec3* kmVec3MultiplyMat4(kmVec3* pOut, const kmVec3* pV,
  const kmMat4* pM);

kmVec3* kmVec3Transform(kmVec3* pOut, const kmVec3* pV1,
  const kmMat4* pM);
kmVec3* kmVec3TransformNormal(kmVec3* pOut, const kmVec3* pV,
  const kmMat4* pM);
kmVec3* kmVec3TransformCoord(kmVec3* pOut, const kmVec3* pV,
  const kmMat4* pM);

kmVec3* kmVec3InverseTransform(kmVec3* pOut, const kmVec3* pV,
  const kmMat4* pM);
kmVec3* kmVec3InverseTransformNormal(kmVec3* pOut, const kmVec3* pVect,
  const kmMat4* pM);

kmVec3* kmVec3GetHorizontalAngle(kmVec3* pOut, const kmVec3 *pIn);

kmVec3* kmVec3RotationToDirection(kmVec3* pOut, const kmVec3* pIn,
  const kmVec3* forwards);
kmVec3* kmVec3ProjectOnToPlane(kmVec3* pOut, const kmVec3* point,
  const kmPlane* plane);
kmVec3* kmVec3ProjectOnToVec3(const kmVec3* pIn, const kmVec3* other,
  kmVec3* projection);

kmVec3* kmVec3Reflect(kmVec3* pOut, const kmVec3* pIn, const kmVec3* normal);

void kmVec3Swap(kmVec3* a, kmVec3* b);
void kmVec3OrthoNormalize(kmVec3* normal, kmVec3* tangent);

]]
