#include <cmath>
#include <vector>
#include <iostream>>

float forwardMultiplyGate(float aA, float aB)
{
	return aA * aB;
}

float forwardAddGate(float aA, float aB)
{
	return aA + aB;
}

float forwardCircuit(float aX, float aY, float aZ)
{
	float q = forwardAddGate(aX, aY);
	
	return forwardMultiplyGate(q, aZ);
}

struct Unit
{
	Unit(float aValue, float aGrad)
		: myValue(aValue)
		, myGrad(aGrad)
	{}

	float myValue;
	float myGrad;
};

struct MultiplyGate
{
	Unit* Forward(Unit* aUnitA, Unit* aUnitB)
	{
		myUnitA = aUnitA;
		myUnitB = aUnitB;

		myOutput = new Unit(aUnitA->myValue * aUnitB->myValue, 0.f);
		return myOutput;
	}

	void Backward()
	{
		myUnitA->myGrad += myUnitB->myValue * myOutput->myGrad;
		myUnitB->myGrad += myUnitA->myValue * myOutput->myGrad;
	}

	Unit* myUnitA;
	Unit* myUnitB;
	Unit* myOutput;
};

struct AddGate
{
	Unit* Forward(Unit* aUnitA, Unit* aUnitB)
	{
		myUnitA = aUnitA;
		myUnitB = aUnitB;

		myOutput = new Unit(aUnitA->myValue + aUnitB->myValue, 0.f);
		return myOutput;
	}

	void Backward()
	{
		myUnitA->myGrad += 1 * myOutput->myGrad;
		myUnitB->myGrad += 1 * myOutput->myGrad;
	}

	Unit* myUnitA;
	Unit* myUnitB;
	Unit* myOutput;
};

struct SigmoidGate
{
	Unit* Forward(Unit* aUnitA)
	{
		myUnitA = aUnitA;

		myOutput = new Unit(Sig(aUnitA->myValue), 0.f);
		return myOutput;
	}

	void Backward()
	{
		float s = Sig(myUnitA->myValue);
		myUnitA->myGrad += (s * (1.f - s)) * myOutput->myGrad;
	}

	float Sig(float aX)
	{
		return 1.f / (1.f + expf(-aX));
	}

	Unit* myUnitA;
	Unit* myOutput;
};

struct Circuit
{
	Unit* Forward(Unit* aX, Unit* aY, Unit* aA, Unit* aB, Unit* aC)
	{
		Unit* _ax = myMulG0.Forward(aA, aX);
		Unit* _by = myMulG1.Forward(aB, aY);

		Unit* _axpby = myAddG0.Forward(_ax, _by);
		Unit* _axpbypc = myAddG1.Forward(_axpby, aC);

		myResultUnit = _axpbypc;
		return myResultUnit;
	}

	void Backward(float aGradientTop)
	{
		myResultUnit->myGrad = aGradientTop;
		myAddG1.Backward();
		myAddG0.Backward();
		myMulG1.Backward();
		myMulG0.Backward();
	}

	MultiplyGate myMulG0;
	MultiplyGate myMulG1;
	AddGate myAddG0;
	AddGate myAddG1;

	Unit* myResultUnit;
};

struct SVM
{
	SVM()
	{
		myA = new Unit(1.f, 0.f);
		myB = new Unit(-2.f, 0.f);
		myC = new Unit(-1.f, 0.f);

		myCircuit = new Circuit();
	}

	Unit* Forward(Unit* aX, Unit *aY)
	{
		myOutUnit = myCircuit->Forward(aX, aY, myA, myB, myC);
		return myOutUnit;
	}

	void Backward(int aLabel)
	{
		myA->myGrad = 0.f;
		myB->myGrad = 0.f;
		myC->myGrad = 0.f;

		float pull = 0.f;
		if (aLabel == 1 && myOutUnit->myValue < 1)
			pull = 1.f;
		if (aLabel == -1 && myOutUnit->myValue > -1)
			pull = -1.f;

		myCircuit->Backward(pull);

		myA->myGrad += -myA->myValue;
		myB->myGrad += -myB->myValue;
	}

	void ParameterUpdate()
	{
		float stepSize = 0.01f;
		myA->myValue += stepSize * myA->myGrad;
		myB->myValue += stepSize * myB->myGrad;
		myC->myValue += stepSize * myC->myGrad;

	}
	void LearnFrom(Unit* aX, Unit* aY, int aLabel)
	{
		Forward(aX, aY);
		Backward(aLabel);
		ParameterUpdate();
	}

	

	Unit* myA;
	Unit* myB;
	Unit* myC;

	Unit* myOutUnit;

	Circuit* myCircuit;
};

struct Input
{
	Input(float aX, float aY)
		: x(aX)
		, y(aY)
	{}

	float x;
	float y;

};
float EvalAccu(const std::vector<Input>& someInputs, const std::vector<int>& someLabels, SVM* anSVM)
{
	int numCorrect = 0;

	for (int i = 0; i < someInputs.size(); ++i)
	{
		const Input& currInput = someInputs[i];
		Unit* x = new Unit(currInput.x, 0.f);
		Unit* y = new Unit(currInput.y, 0.f);
		int realLabel = someLabels[i];

		int predictedLabel = anSVM->Forward(x, y)->myValue > 0 ? 1 : -1;

		if (predictedLabel == realLabel)
			++numCorrect;
	}

	if (numCorrect == 0)
		return 0.f;

	return numCorrect / static_cast<float>(someInputs.size());
}

int main()
{
	/*
	
	Reading and following along here:
	http://karpathy.github.io/neuralnets/
	
	
	*/
	/*
	float x = -2.f;
	float y = 3.f;
	float out = forwardMultiplyGate(x, y);
	float h = 0.0001f;

	float xph = x + h;
	float out2 = forwardMultiplyGate(xph, y);
	float xDerivative = (out2 - out) / h;

	float yph = y + h;
	float out3 = forwardMultiplyGate(x, yph);
	float yDerirative = (out3 - out) / h;


	float xGradient = y;

	float yGradient = x;

	float stepSize = 0.01f;
	float x2 = x + stepSize * xDerivative;
	float y2 = y + stepSize * yDerirative;
	float out4 = forwardMultiplyGate(x2, y2);

	float x3 = x + stepSize * xGradient;
	float y3 = y + stepSize * yGradient;
	float out5 = forwardMultiplyGate(x3, y3);
	*/

	/*
	Unit a(1.f, 0.f);
	Unit b(2.f, 0.f);
	Unit c(-3.f, 0.f);
	Unit x(-1.f, 0.f);
	Unit y(3.f, 0.f);

	MultiplyGate mulG0;
	MultiplyGate mulG1;
	AddGate addG0;
	AddGate addG1;
	SigmoidGate sigG0;

	Unit* ax = mulG0.Forward(&a, &x);
	Unit* by = mulG1.Forward(&b, &y);
	Unit* axpby = addG0.Forward(ax, by);
	Unit* axpbypc = addG1.Forward(axpby, &c);
	Unit* s = sigG0.Forward(axpbypc);

	float result = s->myValue;

	s->myGrad = 1.f;
	sigG0.Backward();
	addG1.Backward();
	addG0.Backward();
	mulG1.Backward();
	mulG0.Backward();

	float stepSize = 0.01f;
	a.myValue += stepSize * a.myGrad;
	b.myValue += stepSize * b.myGrad;
	c.myValue += stepSize * c.myGrad;
	x.myValue += stepSize * x.myGrad;
	y.myValue += stepSize * y.myGrad;


	Unit* ax2 = mulG0.Forward(&a, &x);
	Unit* by2 = mulG1.Forward(&b, &y);
	Unit* axpby2 = addG0.Forward(ax2, by2);
	Unit* axpbypc2 = addG1.Forward(axpby2, &c);
	Unit* s2 = sigG0.Forward(axpbypc2);

	float result2 = s2->myValue;
	*/

	

	std::vector<Input> inputs;
	std::vector<int> labels;

	inputs.push_back(Input(1.2f, 0.7f)); labels.push_back(1);
	inputs.push_back(Input(-0.3, -0.5)); labels.push_back(-1);
	inputs.push_back(Input(3.0, 0.1)); labels.push_back(1);
	inputs.push_back(Input(-0.1, -1.0)); labels.push_back(-1);
	inputs.push_back(Input(-1.0, 1.1)); labels.push_back(-1);
	inputs.push_back(Input(2.1, -3)); labels.push_back(1);

	SVM* svm = new SVM();

	for (int iter = 0; iter < 400; ++iter)
	{
		int i = rand() % inputs.size();

		Input& currInput = inputs[i];
		Unit* x = new Unit(currInput.x, 0.f);
		Unit* y = new Unit(currInput.y, 0.f);
		int label = labels[i];

		svm->LearnFrom(x, y, label);

		if (iter % 25 == 0)
		{
			std::cout << "Accuracy at iter " << iter << ": " << EvalAccu(inputs, labels, svm) << std::endl;
		}
	}

	int apa = 5;
	++apa;
}
