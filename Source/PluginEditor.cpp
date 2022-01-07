/*
  ==============================================================================
	This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
	int x,
	int y,
	int width,
	int height,
	float sliderPosProportional,
	float rotaryStartAngle,
	float rotaryEndAngle,
	juce::Slider& slider)
{
	using namespace juce;
	auto bounds		= Rectangle<float>(x, y, width, height);
	auto enabled	= slider.isEnabled();

	// Draw Circles
	g.setColour(enabled ? Colour(97u, 18u, 167u) : Colours::darkgrey); // base
	g.fillEllipse(bounds);

	g.setColour(enabled ? Colour(255u, 154u, 1u) : Colours::grey); // border
	g.drawEllipse(bounds, 1.f);

	if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
	{
		auto center = bounds.getCentre();
		Path path; // define path for rotation

		Rectangle<float> rec;
		rec.setLeft(center.getX() - 2);
		rec.setRight(center.getX() + 2);
		rec.setTop(bounds.getY());
		rec.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

		path.addRoundedRectangle(rec, 2.f);

		jassert(rotaryStartAngle < rotaryEndAngle); // ROTATION

		auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

		path.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
		g.fillPath(path);

		// Bounding box for text diplay
		g.setFont(rswl->getTextHeight());

		auto text		= rswl->getDisplayString();
		auto strWidth	= g.getCurrentFont().getStringWidth(text);

		rec.setSize(strWidth + 4, rswl->getTextHeight() + 2);
		rec.setCentre(bounds.getCentre());

		g.setColour(enabled ? Colours::black : Colours::darkgrey);
		g.fillRect(rec);

		g.setColour(enabled ? Colours::white : Colours::lightgrey);
		g.drawFittedText(text, rec.toNearestInt(), juce::Justification::centred, 1);
	}
}

void LookAndFeel::drawToggleButton(juce::Graphics& g,
	juce::ToggleButton& toggleButton,
	bool shouldDrawButtonAsHighlighted,
	bool shouldDrawButtonAsDown)
{
	using namespace juce;

	if (auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
	{
		Path powerButton;

		auto bounds = toggleButton.getLocalBounds();
		auto size	= jmin(bounds.getWidth(), bounds.getHeight()) - 6;
		auto r		= bounds.withSizeKeepingCentre(size, size).toFloat();

		float ang = 30.f;
		size -= 6;

		powerButton.addCentredArc(r.getCentreX(), r.getCentreY(), 
								  size * 0.5, size * 0.5, 0.f, 
								  degreesToRadians(ang), 
								  degreesToRadians(360.f - ang), 
								  true);

		powerButton.startNewSubPath(r.getCentreX(), r.getY());
		powerButton.lineTo(r.getCentre());

		PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);

		auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
		g.setColour(color);
		g.strokePath(powerButton, pst);
		g.drawEllipse(r, 2);

	}

	else if (auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
	{
		auto color = !toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);
		g.setColour(color);

		auto bounds = toggleButton.getLocalBounds();
		g.drawRect(bounds);

		g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
	}
}

//==============================================================================

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
	using namespace juce;

	auto startAng		= degreesToRadians(180.f + 45.f);
	auto endAng			= degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
	auto range			= getRange();
	auto sliderBounds	= getSliderBounds();

	// Draw slider & local bounds
	/*g.setColour(Colours::blue);
	g.drawRect(getLocalBounds());
	g.setColour(Colours::red);
	g.drawRect(sliderBounds);*/

	getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(),
									  sliderBounds.getY(),
									  sliderBounds.getWidth(),
									  sliderBounds.getHeight(),
									  jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
									  startAng, endAng, *this);


	auto center		= sliderBounds.toFloat().getCentre();
	auto radius		= sliderBounds.getWidth() * 0.5f;
	auto txtHeight	= getTextHeight();

	g.setColour(Colour(210u, 197u, 232u));
	g.setFont(txtHeight);

	auto numChoices = labels.size();
	for (int labItr = 0; labItr < numChoices; ++labItr)
	{
		auto pos = labels[labItr].pos;
		jassert(0.f <= pos);
		jassert(pos <= 1.f);

		auto ang	= jmap(pos, 0.f, 1.f, startAng, endAng);
		auto ctr	= center.getPointOnCircumference(radius + txtHeight * 0.5f + 1, ang);
		auto str	= labels[labItr].label;

		Rectangle<float> rec;
		rec.setSize(g.getCurrentFont().getStringWidth(str), txtHeight);
		rec.setCentre(ctr);
		rec.setY(rec.getY() + txtHeight);

		g.drawFittedText(str, rec.toNearestInt(), juce::Justification::centred, 1);

	}
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
	auto bounds		= getLocalBounds();
	auto size		= juce::jmin(bounds.getWidth(), bounds.getHeight());

	size -= getTextHeight() * 2; // square around sliders

	juce::Rectangle<int> rec;
	rec.setSize(size, size);
	rec.setCentre(bounds.getCentreX(), 0);
	rec.setY(2); // 2 pixels below top of component

	return rec;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
	if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
		return choiceParam->getCurrentChoiceName();

	juce::String str;
	bool addK = false;

	if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
	{
		float val = getValue();
		if (val > 999.f)
		{
			val /= 1000.f; // if val is > 1000, change to kHz
			addK = true;
		}
		str = juce::String(val, (addK ? 2 : 0));
	}
	else
	{
		// jassertfalse; // nono!
	}

	if (suffix.isNotEmpty())
	{
		str << " ";
		addK ? str << "k" : str << suffix;
		str << suffix;
	}

	return str;
}

//==============================================================================

ResponseCurveComponent::ResponseCurveComponent(TokyoEQAudioProcessor& p) :
	audioProcessor(p),
	leftPathProducer(audioProcessor.leftChannelFifo),
	rightPathProducer(audioProcessor.rightChannelFifo)
{
	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
	{
		param->addListener(this);
	}

	updateChain();
	startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
	{
		param->removeListener(this);
	}
}

void ResponseCurveComponent::updateResponseCurve()
{
	using namespace juce;
	auto responseArea = getAnalysisArea();

	auto w			 = responseArea.getWidth();
	auto& lowcut	 = monoChain.get<ChainPositions::LowCut>();
	auto& peak		 = monoChain.get<ChainPositions::Peak>();
	auto& highcut	 = monoChain.get<ChainPositions::HighCut>();
	auto sampleRate  = audioProcessor.getSampleRate();

	std::vector<double> mags;

	mags.resize(w);

	for (int i = 0; i < w; ++i)
	{
		double mag = 1.f;
		auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

		if (!monoChain.isBypassed<ChainPositions::Peak>())
			mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

		if (!monoChain.isBypassed<ChainPositions::LowCut>())
		{
			if (!lowcut.isBypassed<0>())
				mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<1>())
				mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<2>())
				mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!lowcut.isBypassed<3>())
				mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		if (!monoChain.isBypassed<ChainPositions::HighCut>())
		{
			if (!highcut.isBypassed<0>())
				mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<1>())
				mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<2>())
				mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
			if (!highcut.isBypassed<3>())
				mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		mags[i] = Decibels::gainToDecibels(mag);
	}

	responseCurve.clear();

	const double outputMin = responseArea.getBottom();
	const double outputMax = responseArea.getY();
	auto map = [outputMin, outputMax](double input)
	{
		return jmap(input, -24.0, 24.0, outputMin, outputMax);
	};

	responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

	for (size_t i = 1; i < mags.size(); ++i)
	{
		responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
	}
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
	parametersChanged.set(true);
}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
	juce::AudioBuffer<float> tempIncomingBuffer;

	while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
	{
		if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer)) // if there are more than 0 buffers available
		{
			auto size = tempIncomingBuffer.getNumSamples(); // shifting data
			juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
											  monoBuffer.getReadPointer(0, size), //index size
											  monoBuffer.getNumSamples() - size); // shift

			juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
				tempIncomingBuffer.getReadPointer(0, 0),
				size);

			leftChannelFFTDataGen.produceFFTDataForRendering(monoBuffer, -48.f);
		}
	}

	const auto fftSize  = leftChannelFFTDataGen.getFFTSize();
	const auto binWidth = sampleRate / (double)fftSize;

	while (leftChannelFFTDataGen.getNumAvailableFFTDataBlocks() > 0)
	{
		std::vector<float> fftData;
		if (leftChannelFFTDataGen.getFFTData(fftData)) // pull block
		{
			pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
		}
	}

	while (pathProducer.getNumPathsAvailable())
	{
		pathProducer.getPath(leftChannelFFTPath);
	}

}

void ResponseCurveComponent::timerCallback()
{

	if (shouldShowFFTAnalysis)
	{
		auto fftBounds  = getAnalysisArea().toFloat();
		auto sampleRate = audioProcessor.getSampleRate();

		leftPathProducer.process  (fftBounds, sampleRate);
		rightPathProducer.process (fftBounds, sampleRate);
	}

	if (parametersChanged.compareAndSetBool(false, true))
	{
		DBG("params changed");

		updateChain(); //update monochain
		updateResponseCurve();
	}

	repaint();
}

void ResponseCurveComponent::updateChain()
{
	auto chainSettings = getChainSettings(audioProcessor.apvts);

	monoChain.setBypassed<ChainPositions::LowCut>  (chainSettings.lowCutBypassed);
	monoChain.setBypassed<ChainPositions::Peak>    (chainSettings.peakBypassed);
	monoChain.setBypassed<ChainPositions::HighCut> (chainSettings.highCutBypassed);

	auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
	updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

	auto lowCutCoefficients  = makeLowCutFilter(chainSettings,  audioProcessor.getSampleRate());
	auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

	updateCutFilter(monoChain.get<ChainPositions::LowCut>(),  lowCutCoefficients,  chainSettings.lowCutSlope);
	updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
	using namespace juce;
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(Colours::black);

	drawBackgroundGrid(g);

	auto responseArea = getAnalysisArea();

	if (shouldShowFFTAnalysis)
	{
		auto leftChannelFFTPath = leftPathProducer.getPath();
		leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

		g.setColour(Colour(97u, 18u, 167u)); //purple-
		g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

		auto rightChannelFFTPath = rightPathProducer.getPath();
		rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

		g.setColour(Colour(215u, 201u, 134u));
		g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
	}

	g.setColour(Colours::white);
	g.strokePath(responseCurve, PathStrokeType(2.f));

	Path border;

	border.setUsingNonZeroWinding(false);
	border.addRoundedRectangle(getRenderArea(), 4);
	border.addRectangle(getLocalBounds());

	g.setColour(Colours::black);
	g.fillPath(border);

	drawTextLabel(g);

	g.setColour(Colours::orange);
	g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
}

void  ResponseCurveComponent::drawTextLabel(juce::Graphics& g)
{
	using namespace juce;

	g.setColour(Colours::white); //header labels
	const int fontHeight = 10;
	g.setFont(fontHeight);

	auto renderArea = getAnalysisArea(); // caching info . . .

	auto left			= renderArea.getX();
	auto top			= renderArea.getY();
	auto bottom			= renderArea.getBottom();
	auto width			= renderArea.getWidth();
	auto freqs			= getFrequencies();
	auto xPos			= getXs(freqs, left, width);

	for (auto freqItr = 0; freqItr < freqs.size(); ++freqItr)
	{
		String str;
		bool addK	= false;
		auto freq	= freqs[freqItr];
		auto x		= xPos[freqItr];

		if (freq > 999.f)
		{
			addK	= true;
			freq	/= 1000.f;
		}

		str << freq;
		addK ? str << "k" : str << "Hz";

		auto textWidth = g.getCurrentFont().getStringWidth(str);

		Rectangle<int> rec;
		rec.setSize(textWidth, fontHeight);
		rec.setCentre(x, 0);
		rec.setY(1);

		g.drawFittedText(str, rec, juce::Justification::centred, 1);

	}

	auto gain = getGains();

	for (auto gDb : gain)
	{
		auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

		String str;
		(gDb > 0) ? str << "+" : str << gDb;

		auto textWidth = g.getCurrentFont().getStringWidth(str);

		Rectangle<int> rec;
		rec.setSize(textWidth, fontHeight);
		rec.setX(getWidth() - textWidth);
		rec.setCentre(rec.getCentreX(), y);

		g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::white); // left dB label
		g.drawFittedText(str, rec, juce::Justification::centredLeft, 1);

		str.clear();
		str << (gDb - 24.f);

		rec.setX(1);
		textWidth = g.getCurrentFont().getStringWidth(str);
		rec.setSize(textWidth, fontHeight);
		g.setColour(Colours::white);
		g.drawFittedText(str, rec, juce::Justification::centredLeft, 1); 
	}
}

void ResponseCurveComponent::resized()
{
	using namespace juce;

	responseCurve.preallocateSpace(getWidth() * 3);
	updateResponseCurve();
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
	auto bounds = getLocalBounds();

	bounds.removeFromTop(12);
	bounds.removeFromBottom(2);
	bounds.removeFromLeft(20);
	bounds.removeFromRight(20);

	return bounds; // Box surrounding gridlines
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
	auto bounds = getRenderArea();
	bounds.removeFromTop(4);
	bounds.removeFromBottom(4);
	return bounds;
}

////==============================================================================


std::vector<float> ResponseCurveComponent::getFrequencies()
{
	return std::vector<float>
	{
		20, /*30, 40,*/ 50, 100,
		200, /*300, 400,*/ 500, 1000,
		2000, /*3000, 4000,*/ 5000, 10000,
		20000 // top of hearing range
	};
}

std::vector<float> ResponseCurveComponent::getGains()
{
	return std::vector<float>
	{
		-24, -12, 0, 12, 24
	};
}

std::vector<float> ResponseCurveComponent::getXs(const std::vector<float>& freqs, float left, float width)
{
	std::vector<float> xs;
	for (auto f : freqs)
	{
		auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
		xs.push_back(left + width * normX);
	}

	return xs;
}

void ResponseCurveComponent::drawBackgroundGrid(juce::Graphics& g)
{
	using namespace juce;
	g.setColour(Colours::white);

	auto renderArea = getAnalysisArea();

	auto left		= renderArea.getX();
	auto right		= renderArea.getRight();
	auto top		= renderArea.getY();
	auto bottom		= renderArea.getBottom();
	auto width		= renderArea.getWidth();
	auto freqs		= getFrequencies();
	auto gain		= getGains();
	auto xs			= getXs(freqs, left, width);

	for (auto x : xs)
		g.drawVerticalLine(x, top, bottom);


	for (auto gDb : gain)
	{
		auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

		g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
		g.drawHorizontalLine(y, left, right);
	}
}


////==============================================================================

TokyoEQAudioProcessorEditor::TokyoEQAudioProcessorEditor(TokyoEQAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p),
	peakFreqSlider(*audioProcessor.apvts.getParameter		("Peak Freq"),	   "Hz"),
	peakGainSlider(*audioProcessor.apvts.getParameter		("Peak Gain"),	   "dB"),
	peakQualitySlider(*audioProcessor.apvts.getParameter	("Peak Quality"),  ""),
	lowCutFreqSlider(*audioProcessor.apvts.getParameter		("LowCut Freq"),   "Hz"),
	highCutFreqSlider(*audioProcessor.apvts.getParameter	("HiCut Freq"),	   "Hz"),
	lowCutSlopeSlider(*audioProcessor.apvts.getParameter	("LowCut Slope"),  "dB/Oct"),
	highCutSlopeSlider(*audioProcessor.apvts.getParameter	("HighCut Slope"), "dB/Oct"),


	responseCurveComponent(audioProcessor),

	peakFreqSliderAttachment(audioProcessor.apvts,			"Peak Freq",		peakFreqSlider),
	peakGainSliderAttachment(audioProcessor.apvts,			"Peak Gain",		peakGainSlider),
	peakQualitySliderAttachment(audioProcessor.apvts,		"Peak Quality",		peakQualitySlider),
	lowCutFreqSliderAttachment(audioProcessor.apvts,		"LowCut Freq",		lowCutFreqSlider),
	highCutFreqSliderAttachment(audioProcessor.apvts,		"HighCut Freq",		highCutFreqSlider),
	lowCutSlopeSliderAttachment(audioProcessor.apvts,		"LowCut Slope",		lowCutSlopeSlider),
	highCutSlopeSliderAttachment(audioProcessor.apvts,		"HighCut Slope",	highCutSlopeSlider),

	lowCutBypassedButtonAttachment(audioProcessor.apvts,	"LowCut Bypassed",	lowCutBypassedButton),
	peakBypassButtonAttachment(audioProcessor.apvts,		"Peak Bypassed",	peakBypassButton),
	highCutBypassButtonAttachment(audioProcessor.apvts,		"HighCut Bypassed", highCutBypassButton),
	analyzerEnabledButtonAttachment(audioProcessor.apvts,	"Analyzer Enabled", analyzerEnabledButton)
{

	peakFreqSlider.labels.add(		{ 0.f, "20Hz"	});
	peakFreqSlider.labels.add(		{ 1.f, "20kHz"	});

	peakGainSlider.labels.add(		{ 0.f, "-24dB"	});
	peakGainSlider.labels.add(		{ 1.f, "+24dB"	});

	peakQualitySlider.labels.add(	{ 0.f, "0.1"	});
	peakQualitySlider.labels.add(	{ 1.f, "10.0"	});

	lowCutFreqSlider.labels.add(	{ 0.f, "20Hz"	});
	lowCutFreqSlider.labels.add(	{ 1.f, "20kHz"	});

	highCutFreqSlider.labels.add(	{ 0.f, "20Hz"	});
	highCutFreqSlider.labels.add(	{ 1.f, "20kHz"	});

	lowCutSlopeSlider.labels.add(	{ 0.f, "12"		});
	lowCutSlopeSlider.labels.add(	{ 1.f, "48"		});

	highCutSlopeSlider.labels.add(	{ 0.f, "12"		});
	highCutSlopeSlider.labels.add(	{ 1.f, "48"		});

	for (auto* comp : getComps())
	{
		addAndMakeVisible(comp);
	}

	peakBypassButton.setLookAndFeel(&lnf);
	lowCutBypassedButton.setLookAndFeel(&lnf);
	highCutBypassButton.setLookAndFeel(&lnf);
	analyzerEnabledButton.setLookAndFeel(&lnf);

	auto safePtr = juce::Component::SafePointer<TokyoEQAudioProcessorEditor>(this);
	peakBypassButton.onClick = [safePtr]()
	{
		if (auto* comp = safePtr.getComponent())
		{
			auto bypassed = comp->peakBypassButton.getToggleState();
			comp->peakFreqSlider.setEnabled(!bypassed);
			comp->peakQualitySlider.setEnabled(!bypassed);
		}
	};

	lowCutBypassedButton.onClick = [safePtr]()
	{
		if (auto* comp = safePtr.getComponent())
		{
			auto bypassed = comp->lowCutBypassedButton.getToggleState();
			comp->lowCutFreqSlider.setEnabled(!bypassed);
			comp->lowCutSlopeSlider.setEnabled(!bypassed);
		}
	};

	highCutBypassButton.onClick = [safePtr]()
	{
		if (auto* comp = safePtr.getComponent())
		{
			auto bypassed = comp->highCutBypassButton.getToggleState();
			comp->highCutFreqSlider.setEnabled(!bypassed);
			comp->highCutSlopeSlider.setEnabled(!bypassed);
		}
	};


	analyzerEnabledButton.onClick = [safePtr]()
	{
		if (auto* comp = safePtr.getComponent())
		{
			auto enabled = comp->analyzerEnabledButton.getToggleState();
			comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
		}
	};
	setSize(600, 480); // Window Size
}

TokyoEQAudioProcessorEditor::~TokyoEQAudioProcessorEditor()
{
	peakBypassButton.setLookAndFeel(nullptr);
	lowCutBypassedButton.setLookAndFeel(nullptr);
	highCutBypassButton.setLookAndFeel(nullptr);

	analyzerEnabledButton.setLookAndFeel(nullptr);
}

////==============================================================================

void TokyoEQAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colour(0u, 33u, 71u));
}

void TokyoEQAudioProcessorEditor::resized()
{
	// component positions
	auto bounds = getLocalBounds();
	bounds.removeFromTop(4);

	auto analyzerEnabledArea = bounds.removeFromTop(25);
	analyzerEnabledArea.setWidth(50);
	analyzerEnabledArea.setX(5);
	analyzerEnabledArea.removeFromTop(2);

	analyzerEnabledButton.setBounds(analyzerEnabledArea);

	bounds.removeFromTop(5);

	float hRatio	  = 25.f / 100.f; //JUCE_LIVE_CONSTANT(33) / 100.f;
	auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);

	responseCurveComponent.setBounds(responseArea);

	bounds.removeFromTop(5);

	auto lowCutArea  = bounds.removeFromLeft(bounds.getWidth() * 0.33);
	auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

	lowCutBypassedButton.setBounds(lowCutArea.removeFromTop(25));
	lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
	lowCutSlopeSlider.setBounds(lowCutArea);

	highCutBypassButton.setBounds(highCutArea.removeFromTop(25));
	highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
	highCutSlopeSlider.setBounds(highCutArea);

	peakBypassButton.setBounds(bounds.removeFromTop(25));
	peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
	peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
	peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> TokyoEQAudioProcessorEditor::getComps()
{
	return
	{
		&peakFreqSlider,
		&peakGainSlider,
		&peakQualitySlider,
		&lowCutFreqSlider,
		&highCutFreqSlider,
		&lowCutSlopeSlider,
		&highCutSlopeSlider,
		&responseCurveComponent,

		&lowCutBypassedButton,
		&peakBypassButton,
		&highCutBypassButton,
		&analyzerEnabledButton
	};
}