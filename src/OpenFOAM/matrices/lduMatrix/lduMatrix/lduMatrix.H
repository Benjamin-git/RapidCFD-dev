/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2013 OpenFOAM Foundation
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

Class
    Foam::lduMatrix

Description
    lduMatrix is a general matrix class in which the coefficients are
    stored as three arrays, one for the upper triangle, one for the
    lower triangle and a third for the diagonal.

    Addressing arrays must be supplied for the upper and lower triangles.

    It might be better if this class were organised as a hierachy starting
    from an empty matrix, then deriving diagonal, symmetric and asymmetric
    matrices.

SourceFiles
    lduMatrixATmul.C
    lduMatrix.C
    lduMatrixTemplates.C
    lduMatrixOperations.C
    lduMatrixSolver.C
    lduMatrixPreconditioner.C
    lduMatrixTests.C
    lduMatrixUpdateMatrixInterfaces.C

\*---------------------------------------------------------------------------*/

#ifndef lduMatrix_H
#define lduMatrix_H

#include "lduMesh.H"
#include "primitiveFieldsFwd.H"
#include "FieldField.H"
#include "lduInterfaceFieldPtrsList.H"
#include "typeInfo.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"
#include "solverPerformance.H"
#include "InfoProxy.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of friend functions and operators

class lduMatrix;
Ostream& operator<<(Ostream&, const lduMatrix&);


/*---------------------------------------------------------------------------*\
                           Class lduMatrix Declaration
\*---------------------------------------------------------------------------*/

class lduMatrix
{
    // private data

        //- LDU mesh reference
        const lduMesh& lduMesh_;

        //- Coefficients (not including interfaces)
        scalargpuField *lowerPtr_, *diagPtr_, *upperPtr_;


public:

    //- Abstract base-class for lduMatrix solvers
    class solver
    {
    protected:

        // Protected data

            word fieldName_;
            const lduMatrix& matrix_;
            const FieldField<gpuField, scalar>& interfaceBouCoeffs_;
            const FieldField<gpuField, scalar>& interfaceIntCoeffs_;
            lduInterfaceFieldPtrsList interfaces_;

            //- dictionary of controls
            dictionary controlDict_;

            //- Maximum number of iterations in the solver
            label maxIter_;

            //- Minimum number of iterations in the solver
            label minIter_;

            //- Final convergence tolerance
            scalar tolerance_;

            //- Convergence tolerance relative to the initial
            scalar relTol_;


        // Protected Member Functions

            //- Read the control parameters from the controlDict_
            virtual void readControls();


    public:

        //- Runtime type information
        virtual const word& type() const = 0;


        // Declare run-time constructor selection tables

            declareRunTimeSelectionTable
            (
                autoPtr,
                solver,
                symMatrix,
                (
                    const word& fieldName,
                    const lduMatrix& matrix,
                    const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                    const FieldField<gpuField, scalar>& interfaceIntCoeffs,
                    const lduInterfaceFieldPtrsList& interfaces,
                    const dictionary& solverControls
                ),
                (
                    fieldName,
                    matrix,
                    interfaceBouCoeffs,
                    interfaceIntCoeffs,
                    interfaces,
                    solverControls
                )
            );

            declareRunTimeSelectionTable
            (
                autoPtr,
                solver,
                asymMatrix,
                (
                    const word& fieldName,
                    const lduMatrix& matrix,
                    const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                    const FieldField<gpuField, scalar>& interfaceIntCoeffs,
                    const lduInterfaceFieldPtrsList& interfaces,
                    const dictionary& solverControls
                ),
                (
                    fieldName,
                    matrix,
                    interfaceBouCoeffs,
                    interfaceIntCoeffs,
                    interfaces,
                    solverControls
                )
            );


        // Constructors

            solver
            (
                const word& fieldName,
                const lduMatrix& matrix,
                const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                const FieldField<gpuField, scalar>& interfaceIntCoeffs,
                const lduInterfaceFieldPtrsList& interfaces,
                const dictionary& solverControls
            );

        // Selectors

            //- Return a new solver
            static autoPtr<solver> New
            (
                const word& fieldName,
                const lduMatrix& matrix,
                const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                const FieldField<gpuField, scalar>& interfaceIntCoeffs,
                const lduInterfaceFieldPtrsList& interfaces,
                const dictionary& solverControls
            );



        //- Destructor
        virtual ~solver()
        {}


        // Member functions

            // Access

                const word& fieldName() const
                {
                    return fieldName_;
                }

                const lduMatrix& matrix() const
                {
                    return matrix_;
                }

                 const FieldField<gpuField, scalar>& interfaceBouCoeffs() const
                 {
                     return interfaceBouCoeffs_;
                 }

                 const FieldField<gpuField, scalar>& interfaceIntCoeffs() const
                 {
                     return interfaceIntCoeffs_;
                 }

                 const lduInterfaceFieldPtrsList& interfaces() const
                 {
                     return interfaces_;
                 }


            //- Read and reset the solver parameters from the given stream
            virtual void read(const dictionary&);

            virtual solverPerformance solve
            (
                scalargpuField& psi,
                const scalargpuField& source,
                const direction cmpt=0
            ) const = 0;

            //- Return the matrix norm used to normalise the residual for the
            //  stopping criterion
            scalar normFactor
            (
                const scalargpuField& psi,
                const scalargpuField& source,
                const scalargpuField& Apsi,
                scalargpuField& tmpField
            ) const;
    };


    //- Abstract base-class for lduMatrix smoothers
    class smoother
    {
    protected:

        // Protected data

            word fieldName_;
            const lduMatrix& matrix_;
            const FieldField<gpuField, scalar>& interfaceBouCoeffs_;
            const FieldField<gpuField, scalar>& interfaceIntCoeffs_;
            const lduInterfaceFieldPtrsList& interfaces_;


    public:

        //- Find the smoother name (directly or from a sub-dictionary)
        static word getName(const dictionary&);

        //- Runtime type information
        virtual const word& type() const = 0;


        // Declare run-time constructor selection tables

            declareRunTimeSelectionTable
            (
                autoPtr,
                smoother,
                symMatrix,
                (
                    const word& fieldName,
                    const lduMatrix& matrix,
                    const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                    const FieldField<gpuField, scalar>& interfaceIntCoeffs,
                    const lduInterfaceFieldPtrsList& interfaces,
                    const dictionary& solverControls
                ),
                (
                    fieldName,
                    matrix,
                    interfaceBouCoeffs,
                    interfaceIntCoeffs,
                    interfaces,
                    solverControls
                )
            );

            declareRunTimeSelectionTable
            (
                autoPtr,
                smoother,
                asymMatrix,
                (
                    const word& fieldName,
                    const lduMatrix& matrix,
                    const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                    const FieldField<gpuField, scalar>& interfaceIntCoeffs,
                    const lduInterfaceFieldPtrsList& interfaces,
                    const dictionary& solverControls
                ),
                (
                    fieldName,
                    matrix,
                    interfaceBouCoeffs,
                    interfaceIntCoeffs,
                    interfaces,
                    solverControls
                )
            );


        // Constructors

            smoother
            (
                const word& fieldName,
                const lduMatrix& matrix,
                const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                const FieldField<gpuField, scalar>& interfaceIntCoeffs,
                const lduInterfaceFieldPtrsList& interfaces
            );


        // Selectors

            //- Return a new smoother
            static autoPtr<smoother> New
            (
                const word& fieldName,
                const lduMatrix& matrix,
                const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                const FieldField<gpuField, scalar>& interfaceIntCoeffs,
                const lduInterfaceFieldPtrsList& interfaces,
                const dictionary& solverControls
            );


        //- Destructor
        virtual ~smoother()
        {}


        // Member functions

            // Access

                const word& fieldName() const
                {
                    return fieldName_;
                }

                const lduMatrix& matrix() const
                {
                    return matrix_;
                }

                 const FieldField<gpuField, scalar>& interfaceBouCoeffs() const
                 {
                     return interfaceBouCoeffs_;
                 }

                 const FieldField<gpuField, scalar>& interfaceIntCoeffs() const
                 {
                     return interfaceIntCoeffs_;
                 }

                 const lduInterfaceFieldPtrsList& interfaces() const
                 {
                     return interfaces_;
                 }


            //- Smooth the solution for a given number of sweeps
            virtual void smooth
            (
                scalargpuField& psi,
                const scalargpuField& source,
                const direction cmpt,
                const label nSweeps
            ) const = 0;
    };


    //- Abstract base-class for lduMatrix preconditioners
    class preconditioner
    {
    protected:

        // Protected data

            //- Reference to the base-solver this preconditioner is used with
            const solver& solver_;


    public:

        //- Find the preconditioner name (directly or from a sub-dictionary)
        static word getName(const dictionary&);

        //- Runtime type information
        virtual const word& type() const = 0;


        // Declare run-time constructor selection tables

            declareRunTimeSelectionTable
            (
                autoPtr,
                preconditioner,
                symMatrix,
                (
                    const solver& sol,
                    const dictionary& solverControls
                ),
                (sol, solverControls)
            );

            declareRunTimeSelectionTable
            (
                autoPtr,
                preconditioner,
                asymMatrix,
                (
                    const solver& sol,
                    const dictionary& solverControls
                ),
                (sol, solverControls)
            );


        // Constructors

            preconditioner
            (
                const solver& sol
            )
            :
                solver_(sol)
            {}


        // Selectors

            //- Return a new preconditioner
            static autoPtr<preconditioner> New
            (
                const solver& sol,
                const dictionary& solverControls
            );


        //- Destructor
        virtual ~preconditioner()
        {}


        // Member functions

            //- Read and reset the preconditioner parameters
            //  from the given stream
            virtual void read(const dictionary&)
            {}

            //- Return wA the preconditioned form of residual rA
            virtual void precondition
            (
                scalargpuField& wA,
                const scalargpuField& rA,
                const direction cmpt=0
            ) const = 0;

            //- Return wT the transpose-matrix preconditioned form of
            //  residual rT.
            //  This is only required for preconditioning asymmetric matrices.
            virtual void preconditionT
            (
                scalargpuField& wT,
                const scalargpuField& rT,
                const direction cmpt=0
            ) const
            {
                notImplemented
                (
                    type() +"::preconditionT"
                    "(scalargpuField& wT, const scalargpuField& rT, "
                    "const direction cmpt)"
                );
            }
    };


    // Static data

        // Declare name of the class and its debug switch
        ClassName("lduMatrix");


    // Constructors

        //- Construct given an LDU addressed mesh.
        //  The coefficients are initially empty for subsequent setting.
        lduMatrix(const lduMesh&);

        //- Construct as copy
        lduMatrix(const lduMatrix&);

        //- Construct as copy or re-use as specified.
        lduMatrix(lduMatrix&, bool reUse);

        //- Construct given an LDU addressed mesh and an Istream
        //  from which the coefficients are read
        lduMatrix(const lduMesh&, Istream&);


    //- Destructor
    ~lduMatrix();


    // Member functions

        // Access to addressing

            //- Return the LDU mesh from which the addressing is obtained
            const lduMesh& mesh() const
            {
                return lduMesh_;
            }

            //- Return the LDU addressing
            const lduAddressing& lduAddr() const
            {
                return lduMesh_.lduAddr();
            }

            //- Return the patch evaluation schedule
            const lduSchedule& patchSchedule() const
            {
                return lduAddr().patchSchedule();
            }


        // Access to coefficients

            scalargpuField& lower();
            scalargpuField& diag();
            scalargpuField& upper();

            // Size with externally provided sizes (for constructing with 'fake'
            // mesh in GAMG)

                scalargpuField& lower(const label size);
                scalargpuField& diag(const label nCoeffs);
                scalargpuField& upper(const label nCoeffs);


            const scalargpuField& lower() const;
            const scalargpuField& diag() const;
            const scalargpuField& upper() const;

            bool hasDiag() const
            {
                return (diagPtr_);
            }

            bool hasUpper() const
            {
                return (upperPtr_);
            }

            bool hasLower() const
            {
                return (lowerPtr_);
            }

            bool diagonal() const
            {
                return (diagPtr_ && !lowerPtr_ && !upperPtr_);
            }

            bool symmetric() const
            {
                return (diagPtr_ && (!lowerPtr_ && upperPtr_));
            }

            bool asymmetric() const
            {
                return (diagPtr_ && lowerPtr_ && upperPtr_);
            }


        // operations

            void sumDiag();
            void negSumDiag();

            void sumMagOffDiag(scalargpuField& sumOff) const;

            //- Matrix multiplication with updated interfaces.
            void Amul
            (
                scalargpuField&,
                const tmp<scalargpuField>&,
                const FieldField<gpuField, scalar>&,
                const lduInterfaceFieldPtrsList&,
                const direction cmpt
            ) const;

            //- Matrix transpose multiplication with updated interfaces.
            void Tmul
            (
                scalargpuField&,
                const tmp<scalargpuField>&,
                const FieldField<gpuField, scalar>&,
                const lduInterfaceFieldPtrsList&,
                const direction cmpt
            ) const;


            //- Sum the coefficients on each row of the matrix
            void sumA
            (
                scalargpuField&,
                const FieldField<gpuField, scalar>&,
                const lduInterfaceFieldPtrsList&
            ) const;


            void residual
            (
                scalargpuField& rA,
                const scalargpuField& psi,
                const scalargpuField& source,
                const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                const lduInterfaceFieldPtrsList& interfaces,
                const direction cmpt
            ) const;

            tmp<scalargpuField> residual
            (
                const scalargpuField& psi,
                const scalargpuField& source,
                const FieldField<gpuField, scalar>& interfaceBouCoeffs,
                const lduInterfaceFieldPtrsList& interfaces,
                const direction cmpt
            ) const;


            //- Initialise the update of interfaced interfaces
            //  for matrix operations
            void initMatrixInterfaces
            (
                const FieldField<gpuField, scalar>& interfaceCoeffs,
                const lduInterfaceFieldPtrsList& interfaces,
                const scalargpuField& psiif,
                scalargpuField& result,
                const direction cmpt
            ) const;

            //- Update interfaced interfaces for matrix operations
            void updateMatrixInterfaces
            (
                const FieldField<gpuField, scalar>& interfaceCoeffs,
                const lduInterfaceFieldPtrsList& interfaces,
                const scalargpuField& psiif,
                scalargpuField& result,
                const direction cmpt
            ) const;


            template<class Type>
            tmp<gpuField<Type> > H(const gpuField<Type>&) const;

            template<class Type>
            tmp<gpuField<Type> > H(const tmp<gpuField<Type> >&) const;

            tmp<scalargpuField> H1() const;

            template<class Type>
            tmp<gpuField<Type> > faceH(const gpuField<Type>&) const;

            template<class Type>
            tmp<gpuField<Type> > faceH(const tmp<gpuField<Type> >&) const;


        // Info

            //- Return info proxy.
            //  Used to print matrix information to a stream
            InfoProxy<lduMatrix> info() const
            {
                return *this;
            }


    // Member operators

        void operator=(const lduMatrix&);

        void negate();

        void operator+=(const lduMatrix&);
        void operator-=(const lduMatrix&);

        void operator*=(const scalargpuField&);
        void operator*=(scalar);


    // Ostream operator

        friend Ostream& operator<<(Ostream&, const lduMatrix&);
        friend Ostream& operator<<(Ostream&, const InfoProxy<lduMatrix>&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "lduMatrixTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
