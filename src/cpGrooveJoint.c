/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "chipmunk/chipmunk_private.h"

static void
preStep(cpGrooveJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	// calculate endpoints in worldspace
	cpVect ta = cpTransformPoint(a->transform, joint->grv_a);
	cpVect tb = cpTransformPoint(a->transform, joint->grv_b);

	// calculate axis
	cpVect n = cpTransformVect(a->transform, joint->grv_n);
	cpFloat d = cpvdot(ta, n);
	
	joint->grv_tn = n;
	joint->r2 = cpTransformVect(b->transform, cpvsub(joint->anchorB, b->cog));
	
	// calculate tangential distance along the axis of r2
	cpFloat td = cpvcross(cpvadd(b->p, joint->r2), n);
	// calculate clamping factor and r2
	if(td <= cpvcross(ta, n)){
		joint->clamp = 1.0f;
		joint->r1 = cpvsub(ta, a->p);
	} else if(td >= cpvcross(tb, n)){
		joint->clamp = -1.0f;
		joint->r1 = cpvsub(tb, a->p);
	} else {
		joint->clamp = 0.0f;
		joint->r1 = cpvsub(cpvadd(cpvmult(cpvperp(n), -td), cpvmult(n, d)), a->p);
	}
	
	// Calculate mass tensor
	joint->k = k_tensor(a, b, joint->r1, joint->r2);
	
	// calculate bias velocity
	cpVect delta = cpvsub(cpvadd(b->p, joint->r2), cpvadd(a->p, joint->r1));
	joint->bias = cpvclamp(cpvmult(delta, -bias_coef(joint->constraint.errorBias, dt)/dt), joint->constraint.maxBias);
}

static void
applyCachedImpulse(cpGrooveJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
		
	apply_impulses(a, b, joint->r1, joint->r2, cpvmult(joint->jAcc, dt_coef));
}

static inline cpVect
grooveConstrain(cpGrooveJoint *joint, cpVect j, cpFloat dt){
	cpVect n = joint->grv_tn;
	cpVect jClamp = (joint->clamp*cpvcross(j, n) > 0.0f) ? j : cpvproject(j, n);
	return cpvclamp(jClamp, joint->constraint.maxForce*dt);
}

static void
applyImpulse(cpGrooveJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	cpVect r1 = joint->r1;
	cpVect r2 = joint->r2;
	
	// compute impulse
	cpVect vr = relative_velocity(a, b, r1, r2);

	cpVect j = cpMat2x2Transform(joint->k, cpvsub(joint->bias, vr));
	cpVect jOld = joint->jAcc;
	joint->jAcc = grooveConstrain(joint, cpvadd(jOld, j), dt);
	j = cpvsub(joint->jAcc, jOld);
	
	// apply impulse
	apply_impulses(a, b, joint->r1, joint->r2, j);
}

static cpFloat
getImpulse(cpGrooveJoint *joint)
{
	return cpvlength(joint->jAcc);
}

static const cpConstraintClass klass = {
	(cpConstraintPreStepImpl)preStep,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulse,
	(cpConstraintApplyImpulseImpl)applyImpulse,
	(cpConstraintGetImpulseImpl)getImpulse,
};

cpGrooveJoint *
cpGrooveJointAlloc(void)
{
	return (cpGrooveJoint *)cpcalloc(1, sizeof(cpGrooveJoint));
}

cpGrooveJoint *
cpGrooveJointInit(cpGrooveJoint *joint, cpBody *a, cpBody *b, cpVect groove_a, cpVect groove_b, cpVect anchorB)
{
	cpConstraintInit((cpConstraint *)joint, &klass, a, b);
	
	joint->grv_a = groove_a;
	joint->grv_b = groove_b;
	joint->grv_n = cpvperp(cpvnormalize(cpvsub(groove_b, groove_a)));
	joint->anchorB = anchorB;
	
	joint->jAcc = cpvzero;
	
	return joint;
}

cpConstraint *
cpGrooveJointNew(cpBody *a, cpBody *b, cpVect groove_a, cpVect groove_b, cpVect anchorB)
{
	return (cpConstraint *)cpGrooveJointInit(cpGrooveJointAlloc(), a, b, groove_a, groove_b, anchorB);
}

bool
cpConstraintIsGrooveJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klass);
}

cpVect
cpGrooveJointGetGrooveA(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	return ((cpGrooveJoint *)constraint)->grv_a;
}

void
cpGrooveJointSetGrooveA(cpConstraint *constraint, cpVect value)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	cpGrooveJoint *g = (cpGrooveJoint *)constraint;
	
	g->grv_a = value;
	g->grv_n = cpvperp(cpvnormalize(cpvsub(g->grv_b, value)));
	
	cpConstraintActivateBodies(constraint);
}

cpVect
cpGrooveJointGetGrooveB(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	return ((cpGrooveJoint *)constraint)->grv_b;
}

void
cpGrooveJointSetGrooveB(cpConstraint *constraint, cpVect value)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	cpGrooveJoint *g = (cpGrooveJoint *)constraint;
	
	g->grv_b = value;
	g->grv_n = cpvperp(cpvnormalize(cpvsub(value, g->grv_a)));
	
	cpConstraintActivateBodies(constraint);
}

cpVect
cpGrooveJointGetAnchorB(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	return ((cpGrooveJoint *)constraint)->anchorB;
}

void
cpGrooveJointSetAnchorB(cpConstraint *constraint, cpVect anchorB)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	cpConstraintActivateBodies(constraint);
	((cpGrooveJoint *)constraint)->anchorB = anchorB;
}
