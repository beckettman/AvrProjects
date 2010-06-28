

//*****************************************************************************
//
// Title		: C++ IO for ATMEL AVR
// Author		: Konstantin Chizhov
// Date			: 2010
// Target MCU	: Atmel mega AVR, TINY avr AND Xmega Series
//
//       This code is distributed under the GNU Public License
//       which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#pragma once

#include "iopin.h"

namespace IO
{

	namespace IoPrivate
	{

		template<bool Short> struct SelectSize;

		template<> 
		struct SelectSize<false>
		{
			typedef unsigned int Result;
		};

		template<> 
		struct SelectSize<true>
		{
			typedef unsigned char Result;
		};

	}

// TypeList and corresponding stuff are borrowed from Loki library
// http://sourceforge.net/projects/loki-lib/

// class template NullType - marks the end of type list
class NullType{};

////////////////////////////////////////////////////////////////////////////////
// class template Typelist
// The building block of typelists of any length
// Use it through the LOKI_TYPELIST_NN macros
// Defines nested types:
//     Head (first element, a non-typelist type by convention)
//     Tail (second element, can be another typelist)
////////////////////////////////////////////////////////////////////////////////

    template <class T, class U>
    struct Typelist
    {
       typedef T Head;
       typedef U Tail;
    };

////////////////////////////////////////////////////////////////////////////////
// class template Length
// Computes the length of a typelist
// Invocation (TList is a typelist):
// Length<TList>::value
// returns a compile-time constant containing the length of TList, not counting
//     the end terminator (which by convention is NullType)
////////////////////////////////////////////////////////////////////////////////

        template <class TList> struct Length;
        template <> struct Length<NullType>
        {
            enum { value = 0 };
        };
        
        template <class T, class U>
        struct Length< Typelist<T, U> >
        {
            enum { value = 1 + Length<U>::value };
        };

////////////////////////////////////////////////////////////////////////////////
// class template TypeAt
// Finds the type at a given index in a typelist
// Invocation (TList is a typelist and index is a compile-time integral 
//     constant):
// TypeAt<TList, index>::Result
// returns the type in position 'index' in TList
// If you pass an out-of-bounds index, the result is a compile-time error
////////////////////////////////////////////////////////////////////////////////

        template <class TList, unsigned int index> struct TypeAt;
        
        template <class Head, class Tail>
        struct TypeAt<Typelist<Head, Tail>, 0>
        {
            typedef Head Result;
        };

        template <class Head, class Tail, unsigned int i>
        struct TypeAt<Typelist<Head, Tail>, i>
        {
            typedef typename TypeAt<Tail, i - 1>::Result Result;
        };

////////////////////////////////////////////////////////////////////////////////
// class template Erase
// Erases the first occurence, if any, of a type in a typelist
// Invocation (TList is a typelist and T is a type):
// Erase<TList, T>::Result
// returns a typelist that is TList without the first occurence of T
////////////////////////////////////////////////////////////////////////////////

        template <class TList, class T> struct Erase;
        
        template <class T>                         // Specialization 1
        struct Erase<NullType, T>
        {
            typedef NullType Result;
        };

        template <class T, class Tail>             // Specialization 2
        struct Erase<Typelist<T, Tail>, T>
        {
            typedef Tail Result;
        };

        template <class Head, class Tail, class T> // Specialization 3
        struct Erase<Typelist<Head, Tail>, T>
        {
            typedef Typelist<Head, 
                    typename Erase<Tail, T>::Result>
                Result;
        };
////////////////////////////////////////////////////////////////////////////////
// class template NoDuplicates
// Removes all duplicate types in a typelist
// Invocation (TList is a typelist):
// NoDuplicates<TList, T>::Result
////////////////////////////////////////////////////////////////////////////////

        template <class TList> struct NoDuplicates;
        
        template <> struct NoDuplicates<NullType>
        {
            typedef NullType Result;
        };

        template <class Head, class Tail>
        struct NoDuplicates< Typelist<Head, Tail> >
        {
        private:
            typedef typename NoDuplicates<Tail>::Result L1;
            typedef typename Erase<L1, Head>::Result L2;
        public:
            typedef Typelist<Head, L2> Result;
        };

////////////////////////////////////////////////////////////////////////////////
// class template PW. Pin wrapper
// Holds a Pin class and its bit position in value to read/write
////////////////////////////////////////////////////////////////////////////////

		template<class TPIN, uint8_t POSITION>
		struct PW	//Pin wrapper
		{
			typedef TPIN Pin;
			enum{Position = POSITION};
		};

////////////////////////////////////////////////////////////////////////////////
// class template GetPorts
// Converts list of Pin types to Port types
////////////////////////////////////////////////////////////////////////////////

 		template <class TList> struct GetPorts;
       
        template <> struct GetPorts<NullType>
        {
            typedef NullType Result;
        };

        template <class Head, class Tail>
        struct GetPorts< Typelist<Head, Tail> >
        {
        private:
			typedef typename Head::Pin::Port Port;
            typedef typename GetPorts<Tail>::Result L1;
        public:
            typedef Typelist<Port, L1> Result;
        };

////////////////////////////////////////////////////////////////////////////////
// class template GetPinsWithPort
// Selects pins types form pin list which have givven port
// Assume that TList is type list of PW (pin wrapper) types.
// T - port type to select by.
////////////////////////////////////////////////////////////////////////////////

 		template <class TList, class T> struct GetPinsWithPort;

        template <class T>
        struct GetPinsWithPort<NullType, T>
        {
            typedef NullType Result;
        };

        template <class T, class Tail, uint8_t N, uint8_t M>
        struct GetPinsWithPort<Typelist<PW<TPin<T, N>, M>, Tail>, T>
        {
            // Go all the way down the list removing the type
           typedef Typelist<PW<TPin<T, N>, M>, 
                    typename GetPinsWithPort<Tail, T>::Result>
                Result;
        };

        template <class Head, class Tail, class T>
        struct GetPinsWithPort<Typelist<Head, Tail>, T>
        {
            // Go all the way down the list removing the type
			 typedef typename GetPinsWithPort<Tail, T>::Result Result;
        };

////////////////////////////////////////////////////////////////////////////////
// class template GetPortMask
// Computes port bit mask for pin list
// Assume that TList is type list of PW (pin wrapper) types.
////////////////////////////////////////////////////////////////////////////////

		template <class TList> struct GetPortMask;

        template <> struct GetPortMask<NullType>
        {
            enum{value = 0};
        };

        template <class Head, class Tail>
        struct GetPortMask< Typelist<Head, Tail> >
        {
			enum{value = (1 << Head::Pin::Number) | GetPortMask<Tail>::value};
        };

////////////////////////////////////////////////////////////////////////////////
// class template GetValueMask
// Computes value bit mask for pin list
// Assume that TList is type list of PW (pin wrapper) types.
////////////////////////////////////////////////////////////////////////////////

		template <class TList> struct GetValueMask;

        template <> struct GetValueMask<NullType>
        {
            enum{value = 0};
        };

        template <class Head, class Tail>
        struct GetValueMask< Typelist<Head, Tail> >
        {
			enum{value = (1 << Head::Position) | GetValueMask<Tail>::value};
        };
////////////////////////////////////////////////////////////////////////////////
// class template IsSerial
// Computes if pin list is seqental
// Assume that TList is type list of PW (pin wrapper) types.
////////////////////////////////////////////////////////////////////////////////

		template <class TList> struct IsSerial;

        template <> struct IsSerial<NullType>
        {
            enum{value = 1};
			enum{PinNumber = -1};
			enum{EndOfList = 1};
        };
        template <class Head, class Tail>

        struct IsSerial< Typelist<Head, Tail> >
        {	
		//private:
			typedef IsSerial<Tail> I;
			enum{PinNumber = Head::Pin::Number};
			enum{EndOfList = 0};
		//public:
			enum{value = ((PinNumber == I::PinNumber - 1) && I::value) || I::EndOfList};
        };
////////////////////////////////////////////////////////////////////////////////
// class template Slice
// Gets a slice from typelist.
// Invocation: Slice<Typelist, From, To>::Result
////////////////////////////////////////////////////////////////////////////////
/*
		template <class Pinlist, unsigned int From, unsigned int To> struct Slice;

		template <class Head, class Tail, unsigned int To>
        struct Slice<Typelist<Head, Tail>, 0, To>
        {
            typedef typename TypeAt<Tail, i - 1>::Result Result;
        };

		template <class Head, class Tail, unsigned int From, unsigned int To>
        struct Slice<Typelist<Head, Tail>, From, To>
        {
            typedef typename Slice<Tail, i - 1>::Result Result;
        };

*/
////////////////////////////////////////////////////////////////////////////////
// class template PinWriteIterator
// Iterates througth pin list to compute value to write to their port
// Assume that TList is type list of PW (pin wrapper) types.
////////////////////////////////////////////////////////////////////////////////

		template <class TList> struct PinWriteIterator;

        template <> struct PinWriteIterator<NullType>
        {
			template<class DataType>
			static uint8_t UppendValue(const DataType &value)
			{
				return 0; 
			}

			template<class DataType>
			static inline DataType UppendReadValue(uint8_t portValue, DataType)
			{
				return 0;
			}

        };
        template <class Head, class Tail>

        struct PinWriteIterator< Typelist<Head, Tail> >
        {
			template<class DataType>
			static inline uint8_t UppendValue(const DataType &value)
			{
				if(IsSerial<Typelist<Head, Tail> >::value)
				{
					if((int)Head::Position > (int)Head::Pin::Number)
						return (value >> ((int)Head::Position - (int)Head::Pin::Number)) & 
							GetPortMask<Typelist<Head, Tail> >::value;
					else
						return (value << ((int)Head::Pin::Number - (int)Head::Position)) & 
							GetPortMask<Typelist<Head, Tail> >::value;
				}
				
				uint8_t result=0;

				if((int)Head::Position == (int)Head::Pin::Number)
					result |= value & (1 << Head::Position);
				else 
					if(value & (1 << Head::Position))
						result |= (1 << Head::Pin::Number);

				return result | PinWriteIterator<Tail>::UppendValue(value);
			}

			template<class DataType>
			static inline DataType UppendReadValue(uint8_t portValue, DataType dummy)
			{
				if(IsSerial<Typelist<Head, Tail> >::value)
				{
					if((int)Head::Position > (int)Head::Pin::Number)
						return (portValue >> ((int)Head::Position - (int)Head::Pin::Number)) & 
							GetValueMask<Typelist<Head, Tail> >::value;
					else
						return (portValue << ((int)Head::Pin::Number - (int)Head::Position)) & 
							GetValueMask<Typelist<Head, Tail> >::value;
				}

				DataType value=0;
				if((int)Head::Position == (int)Head::Pin::Number)
					value |= portValue & (1 << Head::Position);
				else 
					if(portValue & (1 << Head::Pin::Number))
						value |= (1 << Head::Position);

				return value | PinWriteIterator<Tail>::UppendReadValue(portValue, dummy);
			}
        };

////////////////////////////////////////////////////////////////////////////////
// class template PortWriteIterator
// Iterates througth port list and write value to them
// Assume that PinList is type list of PW (pin wrapper) types.
// And PortList is type list of port types.
////////////////////////////////////////////////////////////////////////////////

		template <class PortList, class PinList> struct PortWriteIterator;

        template <class PinList> struct PortWriteIterator<NullType, PinList>
        {
			template<class DataType>
			static void Write(DataType value)
			{   }

			template<class DataType>
			static void Set(DataType value)
			{   }

			template<class DataType>
			static void Clear(DataType value)
			{   }

			template<class DataType>
			static DataType PinRead(DataType)
			{   
				return 0;
			}
			
			template<class DataType>
			static void DirWrite(DataType value)
			{	}

			template<class DataType>
			static void DirSet(DataType value)
			{   }

			template<class DataType>
			static void DirClear(DataType value)
			{   }

			template<class DataType>
			static DataType OutRead(DataType dummy)
			{
				return 0;	
			}
        };

        template <class Head, class Tail, class PinList>
        struct PortWriteIterator< Typelist<Head, Tail>, PinList>
        {
			//pins on current port
			typedef typename GetPinsWithPort<PinList, Head>::Result Pins;
			enum{Mask = GetPortMask<Pins>::value};
			typedef Head Port; //Head points to current port i the list.
			
			template<class DataType>
			static void Write(DataType value)
			{   
				uint8_t result = PinWriteIterator<Pins>::UppendValue(value);
				if((int)Length<Pins>::value == (int)Port::Width)// whole port		
					Port::Write(result);
				else
					Port::Write((Port::Read() & ~Mask) | result);

				PortWriteIterator<Tail, PinList>::Write(value);
			}

			template<class DataType>
			static void Set(DataType value)
			{   
				uint8_t result = PinWriteIterator<Pins>::UppendValue(value);
				Port::Set(result);
				
				PortWriteIterator<Tail, PinList>::Set(value);
			}

			template<class DataType>
			static void Clear(DataType value)
			{   
				uint8_t result = PinWriteIterator<Pins>::UppendValue(value);
				Port::Clear(result);
				
				PortWriteIterator<Tail, PinList>::Clear(value);
			}

			template<class DataType>
			static void DirWrite(DataType value)
			{   
				uint8_t result = PinWriteIterator<Pins>::UppendValue(value);
				if((int)Length<Pins>::value == (int)Port::Width)
					Port::dir() = result;
				else
					Port::DirWrite((Port::DirRead() & ~Mask) | result);

				PortWriteIterator<Tail, PinList>::DirWrite(value);
			}

			template<class DataType>
			static void DirSet(DataType value)
			{   
				uint8_t result = PinWriteIterator<Pins>::UppendValue(value);
				Port::DirSet(result);
				
				PortWriteIterator<Tail, PinList>::DirSet(value);
			}

			template<class DataType>
			static void DirClear(DataType value)
			{   
				uint8_t result = PinWriteIterator<Pins>::UppendValue(value);
				Port::DirClear(result);
				
				PortWriteIterator<Tail, PinList>::DirClear(value);
			}
			
			template<class DataType>
			static DataType PinRead(DataType dummy)
			{   
				uint8_t portValue = Port::PinRead();
				DataType value = PinWriteIterator<Pins>::UppendReadValue(portValue, dummy);	
				return value | PortWriteIterator<Tail, PinList>::PinRead(dummy);
			}

			template<class DataType>
			static DataType OutRead(DataType dummy)
			{   
				uint8_t portValue = Port::Read();
				DataType value = PinWriteIterator<Pins>::UppendReadValue(portValue, dummy);	
				return value | PortWriteIterator<Tail, PinList>::OutRead(dummy);
			}

        };

////////////////////////////////////////////////////////////////////////////////
// class template PinSet
// Holds implimentation of pin list manipulations.
// Pins from list are grouped by their port and group read/write operation is 
// performed on each port.
////////////////////////////////////////////////////////////////////////////////

		template<class PINS>
		struct PinSet
		{
		private:
			typedef typename GetPorts<PINS>::Result PinsToPorts;
			typedef typename NoDuplicates<PinsToPorts>::Result Ports; 
		public:
			enum{Length = Length<PINS>::value};
			typedef typename IoPrivate::SelectSize<Length < 8>::Result DataType;

			template<uint8_t PIN>
			class Pin :public TypeAt<PINS, PIN>::Result::Pin
			{
			};

			static void Write(DataType value)
			{
				PortWriteIterator<Ports, PINS>::Write(value);
			}

			static DataType Read()
			{	
				typedef PortWriteIterator<Ports, PINS> iter;
				return iter::OutRead(DataType(0));
			}
			static void Set(DataType value)
			{
				PortWriteIterator<Ports, PINS>::Set(value);
			}

			static void Clear(DataType value)
			{
				PortWriteIterator<Ports, PINS>::Clear(value);
			}

			static DataType PinRead()
			{	
				typedef PortWriteIterator<Ports, PINS> iter;
				return iter::PinRead(DataType(0));
			}

			static void DirWrite(DataType value)
			{
				PortWriteIterator<Ports, PINS>::DirWrite(value);
			}
			
			static void DirSet(DataType value)
			{
				PortWriteIterator<Ports, PINS>::DirSet(value);
			}

			static void DirClear(DataType value)
			{
				PortWriteIterator<Ports, PINS>::DirClear(value);
			}
		};

////////////////////////////////////////////////////////////////////////////////
// class template MakePinList
// This class is used to generate PinList and associate each pin in the list with
// its bit position in value to be Write to and Read from pin list.
////////////////////////////////////////////////////////////////////////////////

		template
        <	
			int Position,
            typename T1  = NullType, typename T2  = NullType, typename T3  = NullType,
            typename T4  = NullType, typename T5  = NullType, typename T6  = NullType,
            typename T7  = NullType, typename T8  = NullType, typename T9  = NullType,
            typename T10 = NullType, typename T11 = NullType, typename T12 = NullType,
            typename T13 = NullType, typename T14 = NullType, typename T15 = NullType,
            typename T16 = NullType, typename T17 = NullType
        > 
        struct MakePinList
        {
        private:
            typedef typename MakePinList
            <
				Position + 1,
                T2 , T3 , T4 , 
                T5 , T6 , T7 , 
                T8 , T9 , T10, 
                T11, T12, T13,
                T14, T15, T16, 
                T17
            >
            ::Result TailResult;
			enum{PositionInList = Position};
        public:
            typedef Typelist< PW<T1, PositionInList>, TailResult> Result;

        };

        template<int Position>
        struct MakePinList<Position>
        {
            typedef NullType Result;
        };

////////////////////////////////////////////////////////////////////////////////
// class template PinList
// Represents generic set of IO pins that could be used like a virtual port.
// It can be composed from any number of pins from 1 to 16 from any IO port present in selected device
// (the last T17 type in PinList is a end of list marker).
// It can be used like this:
//		typedef PinList<Pa0, Pa1, Pa2, Pa3, Pb5, Pb4, Pb2> pins;
//		pins::Write(someValue);

////////////////////////////////////////////////////////////////////////////////

		template
        <	
            typename T1  = NullType, typename T2  = NullType, typename T3  = NullType,
            typename T4  = NullType, typename T5  = NullType, typename T6  = NullType,
            typename T7  = NullType, typename T8  = NullType, typename T9  = NullType,
            typename T10 = NullType, typename T11 = NullType, typename T12 = NullType,
            typename T13 = NullType, typename T14 = NullType, typename T15 = NullType,
            typename T16 = NullType, typename T17 = NullType
        > 
        struct PinList: public PinSet
			<
				typename MakePinList
				<	0,	T1,
					T2 , T3 , T4 , 
                	T5 , T6 , T7 , 
                	T8 , T9 , T10, 
                	T11, T12, T13,
                	T14, T15, T16, 
                	T17
				>::Result
			>
        {	};

}