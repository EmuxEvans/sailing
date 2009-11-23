#pragma once

//
#define MY_SUFFIX				0
#define MY_TEMPLATE			
#define MY_TEMPLATE_PARAMS		
#define MY_TEMPLATE_ARGS		
#define MY_PARAMS				
#define MY_ARGS				
#define MY_T_TEMPLATE_PARAMS	<typename T>
#include "DelegateImpl.h"

//
#define MY_SUFFIX				1
#define MY_TEMPLATE			template
#define MY_TEMPLATE_PARAMS		<typename TP1>
#define MY_TEMPLATE_ARGS		<TP1>
#define MY_PARAMS				TP1 p1
#define MY_ARGS				p1
#define MY_T_TEMPLATE_PARAMS	<typename T, typename TP1>
#include "DelegateImpl.h"

//
#define MY_SUFFIX				2
#define MY_TEMPLATE			template
#define MY_TEMPLATE_PARAMS		<typename TP1, typename TP2>
#define MY_TEMPLATE_ARGS		<TP1, TP2>
#define MY_PARAMS				TP1 p1, TP2 p2
#define MY_ARGS				p1, p2
#define MY_T_TEMPLATE_PARAMS	<typename T, typename TP1, typename TP2>
#include "DelegateImpl.h"

//
#define MY_SUFFIX				3
#define MY_TEMPLATE			template
#define MY_TEMPLATE_PARAMS		<typename TP1, typename TP2, typename TP3>
#define MY_TEMPLATE_ARGS		<TP1, TP2, TP3>
#define MY_PARAMS				TP1 p1, TP2 p2, TP3 p3
#define MY_ARGS				p1, p2, p3
#define MY_T_TEMPLATE_PARAMS	<typename T, typename TP1, typename TP2, typename TP3>
#include "DelegateImpl.h"

//
#define MY_SUFFIX				4
#define MY_TEMPLATE			template
#define MY_TEMPLATE_PARAMS		<typename TP1, typename TP2, typename TP3, typename TP4>
#define MY_TEMPLATE_ARGS		<TP1, TP2, TP3, TP4>
#define MY_PARAMS				TP1 p1, TP2 p2, TP3 p3, TP4 p4
#define MY_ARGS				p1, p2, p3, p4
#define MY_T_TEMPLATE_PARAMS	<typename T, typename TP1, typename TP2, typename TP3, typename TP4>
#include "DelegateImpl.h"

//
#define MY_SUFFIX				5
#define MY_TEMPLATE			template
#define MY_TEMPLATE_PARAMS		<typename TP1, typename TP2, typename TP3, typename TP4, typename TP5>
#define MY_TEMPLATE_ARGS		<TP1, TP2, TP3, TP4, TP5>
#define MY_PARAMS				TP1 p1, TP2 p2, TP3 p3, TP4 p4, TP5 p5
#define MY_ARGS				p1, p2, p3, p4, p5
#define MY_T_TEMPLATE_PARAMS	<typename T, typename TP1, typename TP2, typename TP3, typename TP4, typename TP5>
#include "DelegateImpl.h"
