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
preStep(cpRatchetJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	cpFloat angle = joint->angle;
	cpFloat phase = joint->phase;
	cpFloat ratchet = joint->ratchet;
	
	cpFloat delta = b->a - a->a;
	cpFloat diff = angle - delta;
	cpFloat pdist = 0.0f;
	
	if(diff*ratchet > 0.0f){
		pdist = diff;
	} else {
		joint->angle = cpffloor((delta - phase)/ratchet)*ratchet + phase;
	}
	
	// calculate moment of inertia coefficient.
	joint->iSum = 1.0f/(a->i_inv + b->i_inv);
	
	// calculate bias velocity
	cpFloat maxBias = joint->constraint.maxBias;
	joint->bias = cpfclamp(-bias_coef(joint->constraint.errorBias, dt)*pdist/dt, -maxBias, maxBias);

	// If the bias is 0, the joint is not at a limit. Reset the impulse.
	if(!joint->bias) joint->jAcc = 0.0f;
}

static void
applyCachedImpulse(cpRatchetJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	cpFloat j = joint->jAcc*dt_coef;
	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static void
applyImpulse(cpRatchetJoint *joint, cpFloat dt)
{
	if(!joint->bias) return; // early exit

	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	// compute relative rotational velocity
	cpFloat wr = b->w - a->w;
	cpFloat ratchet = joint->ratchet;
	
	cpFloat jMax = joint->constraint.maxForce*dt;
	
	// compute normal impulse	
	cpFloat j = -(joint->bias + wr)*joint->iSum;
	cpFloat jOld = joint->jAcc;
	joint->jAcc = cpfclamp((jOld + j)*ratchet, 0.0f, jMax*cpfabs(ratchet))/ratchet;
	j = joint->jAcc - jOld;
	
	// apply impulse
	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static cpFloat
getImpulse(cpRatchetJoint *joint)
{
	return cpfabs(joint->jAcc);
}

static const cpConstraintClass klass = {
	(cpConstraintPreStepImpl)preStep,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulse,
	(cpConstraintApplyImpulseImpl)applyImpulse,
	(cpConstraintGetImpulseImpl)getImpulse,
};

cpRatchetJoint *
cpRatchetJointAlloc(void)
{
	return (cpRatchetJoint *)cpcalloc(1, sizeof(cpRatchetJoint));
}

cpRatchetJoint *
cpRatchetJointInit(cpRatchetJoint *joint, cpBody *a, cpBody *b, cpFloat phase, cpFloat ratchet)
{
	cpConstraintInit((cpConstraint *)joint, &klass, a, b);
	
	joint->angle = 0.0f;
	joint->phase = phase;
	joint->ratchet = ratchet;
	
	// STATIC_BODY_CHECK
	joint->angle = (b ? b->a : 0.0f) - (a ? a->a : 0.0f);
	
	return joint;
}

cpConstraint *
cpRatchetJointNew(cpBody *a, cpBody *b, cpFloat phase, cpFloat ratchet)
{
	return (cpConstraint *)cpRatchetJointInit(cpRatchetJointAlloc(), a, b, phase, ratchet);
}

bool
cpConstraintIsRatchetJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klass);
}

cpFloat
cpRatchetJointGetAngle(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	return ((cpRatchetJoint *)constraint)->angle;
}

void
cpRatchetJointSetAngle(cpConstraint *constraint, cpFloat angle)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	cpConstraintActivateBodies(constraint);
	((cpRatchetJoint *)constraint)->angle = angle;
}

cpFloat
cpRatchetJointGetPhase(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	return ((cpRatchetJoint *)constraint)->phase;
}

void
cpRatchetJointSetPhase(cpConstraint *constraint, cpFloat phase)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	cpConstraintActivateBodies(constraint);
	((cpRatchetJoint *)constraint)->phase = phase;
}
cpFloat
cpRatchetJointGetRatchet(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	return ((cpRatchetJoint *)constraint)->ratchet;
}

void
cpRatchetJointSetRatchet(cpConstraint *constraint, cpFloat ratchet)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	cpConstraintActivateBodies(constraint);
	((cpRatchetJoint *)constraint)->ratchet = ratchet;
}
