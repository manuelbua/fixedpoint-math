## Fixed-point arithmetic in C++

Some pet-projects ago, i got hooked up by fixed-point arithmetic and its usefullness on the mobile platform where GPUs and multiple cores weren't that ubiquitous.

While experimenting with the traditional implementations such as [this one](http://people.ece.cornell.edu/land/courses/ece4760/Math/) or this [other one](http://stackoverflow.com/questions/79677/whats-the-best-way-to-do-fixed-point-math), i wanted to try an object-oriented approach and this is the result.

At that time, a more traditional-style implementation (#defines-based) was really faster than the object-oriented one, obviously the overhead was larger than the gain, but it was an interesting experiment nonetheless.

### Usage

The class itself predefines both the 24.8 and the 16.16 formats:
	typedef FixedPoint< 8, LowPrecision,  LowPrecision> fixed8_t;
	typedef FixedPoint<16, HighPrecision, HighPrecision> fixed16_t;

An usage example coult be a generic, datatype-unaware *matrix* implementation:

	template<class Number>
	class matrix {
		private:
			Number m[4][4];

		public:
			matrix();

		// matrix indexing
		inline Number operator()(const int,const int);

		// matrix operations
		matrix<Number> operator*(const matrix<Number>);
		matrix<Number> operator/(const matrix<Number>);
	};

Then you should be able to declare your test *matrix* types like this:

	matrix<float_t> float_matrix;
	matrix<fixed16_t> fixed_matrix;