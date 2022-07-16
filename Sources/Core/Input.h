#pragma once

enum class Key {
	Unknow =       -1,
	Space =        32,
	Apostrophe =   39, // '
	Comma =        44, // ,
	Minus =        45, // -
	Period =       46, // .
	Slash =        47, // /
				   
	N_0 =          48,
	N_1 =          49,
	N_2 =          50,
	N_3 =          51,
	N_4 =          52,
	N_5 =          53,
	N_6 =          54,
	N_7 =          55,
	N_8 =          56,
	N_9 =          57,
				   
	Semicolon =    59, // ;
	Equal =        61, // =
				   
	A =            65,
	B =            66,
	C =            67,
	D =            68,
	E =            69,
	F =            70,
	G =            71,
	H =            72,
	I =            73,
	J =            74,
	K =            75,
	L =            76,
	M =            77,
	N =            78,
	O =            79,
	P =            80,
	Q =            81,
	R =            82,
	S =            83,
	T =            84,
	U =            85,
	V =            86,
	W =            87,
	X =            88,
	Y =            89,
	Z =            90,
				   
	LeftBracket =  91, // [
	BackSlash =    92, /* \ */
	RightBracket = 93, // ]
	GraveAccent =  96, // `
				   
	World1 =       161,
	World2 =       162,
				   
	Escape =       256,
	Enter  =       257,
	Tab =          258,
	Backspace =    259,
	Insert =       260,
	Delete =       261,
				   
	Right =        262,
	Left =         263,
	Down =         264,
	Up =           265,
				   
	PageUp =       266,
	PageDown =     267,
	Home =         268,
	End =          269,
				   
	CapsLock =	   280,
	ScrollLock =   281,
	NumLock =      282,
	PrintScreen =  283,
	Pause =        284,
				   
	F1 =           290,
	F2 =           291,
	F3 =           292,
	F4 =           293,
	F5 =           294,
	F6 =           295,
	F7 =           296,
	F8 =           297,
	F9 =           298,
	F10 =          299,
	F11 =          300,
	F12 =          301,
	F13 =          302,
	F14 =          303,
	F15 =          304,
	F16 =          305,
	F17 =          306,
	F18 =          307,
	F19 =          308,
	F20 =          309,
	F21 =          310,
	F22 =          311,
	F23 =          312,
	F24 =          313,
	F25 =          314,
				   
	Kp_0 =         320,
	Kp_1 =         321,
	Kp_2 =         322,
	Kp_3 =         323,
	Kp_4 =         324,
	Kp_5 =         325,
	Kp_6 =         326,
	Kp_7 =         327,
	Kp_8 =         328,
	Kp_9 =         329,
				   
	Kp_Decimal =   330,
	Kp_Divide =    331,
	Kp_Multiply =  332,
	kp_Subtract =  333,
	Kp_Add =       334,
	Kp_Enter =     335,
	Kp_Equal =     336,
				   
	LeftShift =    340,
	LeftControl =  341,
	LeftAlt =      342,
	LeftSuper =    343,
	RightShift =   344,
	RightControl = 345,
	RightAlt =     346,
	RightSuper =   347,
				   
	Menu =         348,

	__LAST__ = Menu
};

enum class Button {
	Left =   0,
	Right = 1,
	Middle = 2,
};

enum class ButtonState {
	Released,
	Pressing,
	Pressed,
	Releasing
};


class Input {

public:
	static bool IsKeyPressed(Key key);
	static bool IsKeyDown(Key key);
	static bool IsButtonPressed(Button button);
};