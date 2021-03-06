/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

InClass
    Foam::primitiveFields

Description
    Forward declarations of the specialisations of Field\<T\> for
    scalar, vector and tensor.

\*---------------------------------------------------------------------------*/

#ifndef primitiveFieldsFwd_H
#define primitiveFieldsFwd_H

#include "fieldTypes.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type> class Field;
template<class Type> class gpuField;

typedef Field<label> labelField;
typedef gpuField<label> labelgpuField;
typedef Field<scalar> scalarField;
typedef gpuField<scalar> scalargpuField;
typedef Field<vector> vectorField;
typedef gpuField<vector> vectorgpuField;
typedef Field<sphericalTensor> sphericalTensorField;
typedef gpuField<sphericalTensor> sphericalTensorgpuField;
typedef Field<symmTensor> symmTensorField;
typedef gpuField<symmTensor> symmTensorgpuField;
typedef Field<tensor> tensorField;
typedef gpuField<tensor> tensorgpuField;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
