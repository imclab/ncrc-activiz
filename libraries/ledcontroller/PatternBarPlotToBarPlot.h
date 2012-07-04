// //
// //  PatternBarPlotToBarPlot.h
// //  
// //
// //  Created by mac on 6/21/12.
// //  Author: Chuan-Che Huang. chuanche@umich.edu
// //
// 
// #pragma once
// 
// #include "Namespace.h"
// #include "Color.h"
// #include "PatternSineWave.h"
// #include "Interval.h"
// 
// LED_CONTROLLER_NAMESPACE_ENTER
// 
// class PatternBarPlotToBarPlot : public PatternSineWave
// {
// private:
// 	byte startPosition;
// 	byte endPosition;
// 	Interval moveInterval;
// 	Interval eachActionInterval;
// 	byte currentPosition;
// 	bool reverse;
// 	Color bgColor;
// 	Color barColor;
// 	byte increment;
// 
// 	void advance();
// public:
// 	/*
// 	*	Create a LED animation, the y-value for barplot will start at
// 	*	"startPosition", and then go to "endPosition" given the
// 	*	moveInterval it has.
// 	*/
// 	PatternBarPlotToBarPlot(byte startPosition, byte endPosition, 
// 		const Color& bgColor, const Color& barColor, unsigned long milliSecForInterval );
// 	
// 	/**
// 	*	@return whether a single action is being performed
// 	*	multiple actions = one total animation
// 	*/
// 	bool update();
// 	
// 	void apply(Color* stripColors);
// 	
// 	/*
// 	*	@return true if the whole animation is finished 
// 	*	which means all the actions are finished
// 	*	one action = turn on/off one individual led
// 	*/
// 	bool isExpired();
// 	/**
// 	*	Setter and Getter for reverse
// 	*/
// 	void setReverse();
// 	bool isReverse();
// };
// 
// LED_CONTROLLER_NAMESPACE_EXIT