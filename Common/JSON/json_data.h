#pragma once

//-----------------------------------------------------------------------------
// Code support required for serialization.
class JSONDATA
{
public:
	// to be JSON'ised
	std::string text;
public:
	// each class requires a public serialize function
	void serialize(JSON::Adapter& adapter)
	{
		// this pattern is required 
		JSON::Class root(adapter, "JSONDATA");
		// this is the last member variable we serialize so use the _T variant
		JSON_T(adapter, text);
	}
};
