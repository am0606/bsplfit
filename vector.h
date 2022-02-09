    // vector.h  -  тип вектор и работа с векторами.

#ifndef vectorincluded

#include <iostream>
#include <cctype>
#include <cmath>
#include <cassert>

template <int N>
  class vector
  {double components[N];   // Декартовы компоненты вектора.

     vector<N> &makezero()   // Зануление компонент вектора.
      {
         for (int i = 0; i < N; i++)
            components[i] = 0.;
         return *this;
      }

     vector<N> &copy(const vector<N> &source)
      {
         for (int i = 0; i < N; i++)
            components[i] = source.components[i];
         return *this;
      }

     public:

     enum vectorindex
       {X = 0, Y = 1, Z = 2};      // Индексы для трехмерных векторов.

     vector(double a, double b)
       {assert(N == 2);
        components[0] = a;
        components[1] = b;
       }

     vector(double a, double b, double c)
      {
         assert(N == 3);
         components[0] = a;
         components[1] = b;
         components[2] = c;
      }

     vector(int)     // Задание нулевого вектора.
      {
         makezero();
      }

     vector(vectorindex r)   // Задание базисного вектора.
      {
         makezero();
         components[r] = 1.;
      }

     vector() {}

     vector(const vector<N> &source)
      {
         copy(source);
      }

     // 1. Унарные + и -.

     vector <N> operator-() const
      {
        vector<N> rezult;

        for (int i = 0; i < N; i++)
           rezult.components[i] = - components[i];
        return(rezult);
      }

     vector <N> operator+() const
      {
         return(*this);
      }

     // 2. Операции присваивания.

     vector<N> & operator+=(const vector<N> &x)
      {
        for (int i = 0; i < N; i++)
           components[i] += x.components[i];
        return(*this);
      }

     vector<N> & operator-=(const vector<N> &x)
      {
        for (int i = 0; i < N; i++)
           components[i] -= x.components[i];
        return(*this);
      }

     vector<N> & operator*=(const double x)
      {
        for (int i = 0; i < N; i++)
           components[i] *= x;
        return(*this);
      }

     vector<N> & operator/=(const double x)
      {

         for (int i = 0; i < N; i++)
            components[i] /= x;
         return(*this);
      }

     // 3. Мультипликативные операции.

     inline friend double operator*(const vector<N> &a, const vector<N> &b)
        // Скалярное произведение векторов.
      {
          double tttmp;

          for (int i = 0, tttmp = 0.; i < N; i++)
             tttmp += a.components[i] * b.components[i];
          return(tttmp);
      }

     inline friend vector<N> operator*(const double c, const vector<N> &a)
       // Умножение вектора на число.
      {
         vector<N> rezult;

         for (int i = 0; i < N; i++)
            rezult.components[i] = a.components[i] * c;
         return(rezult);
      }

     inline friend vector<N> operator*(const vector<N> &a, const double c)
      {
         vector<N> rezult;

         for (int i = 0; i < N; i++)
            rezult.components[i] = a.components[i] * c;
         return(rezult);
      }

     inline friend vector<N> operator/(const vector<N> &a, const double c)
       // Деление вектора на число.
      {
         vector<N> rezult;

         for (int i = 0; i < N; i++)
            rezult.components[i] = a.components[i] / c;
         return(rezult);
      }

     // 4. Сложение и вычитание.

     inline friend vector<N> operator+(const vector<N> &a, const vector<N> &b)
      {
         vector<N> rezult;

         for (int i = 0; i < N; i++)
            rezult.components[i] = a.components[i] + b.components[i];
         return(rezult);
      }

     inline friend vector<N> operator-(const vector<N> &a, const vector<N> &b)
      {
         vector<N> rezult;

         for (int i = 0; i < N; i++)
            rezult.components[i] = a.components[i] - b.components[i];
         return(rezult);
      }

     // 5. Сравнение двух векторов.

     inline friend int operator==(const vector<N> &a, const vector<N> &b)
      {
         for (int i = 0; i < N; i++)
            if (a.components[i] != b.components[i])
              return(0);
         return(1);
      }

     inline friend int operator!=(const vector<N> &a, const vector<N> &b)
      {
         for (int i = 0; i < N; i++)
            if (a.components[i] != b.components[i])
              return(1);
         return(0);
      }
	 
     // 6. Ввод и вывод.

     inline friend std::istream &operator>>(std::istream &stream, vector<N> &a)
      {
         // Вектор должен нормально считываться как в случае, когда
         // во входном потоке присутствуют лишь N чисел, разделенных
         // пробельными символами, так и в случае, когда значения
         // компонент, помимо пробельных символов, разделены еще и
         // запятыми, а все вместе заключено в круглые скобки.

         char c;
         bool in_brackets = true;
         while (stream.get(c) && c != '(')
           if (!isspace(c))
             {stream.putback(c);
              in_brackets = false;
              break;
             }

         for (int i = 0; i < N; i++)
            {while (stream.get(c) && c != ',')
               if (!isspace(c))
                 {stream.putback(c);
                  break;
                 }

             stream >> a.components[i];
            }

         while (in_brackets && stream.get(c) && c != ')')
           if (!isspace(c))
             {stream.putback(c);
              break;
             }

         return(stream);
      }

     inline friend std::ostream &operator<<(std::ostream &stream, const vector<N> &a)
      {
		 stream << '(';

         for (int i = 0; i < N; i++)
            stream << a.components[i] << ((i < N-1) ? ", " : ")");
         return(stream);
      }

     // 7. Некоторые функции векторов.

     // Квадрат длины вектора.

     inline friend double sqs(const vector<N> &a)
      {
         return(a * a);
      }

     // Длина вектора.

     inline friend double fabs(const vector<N> &a)
      {
         return(sqrt(a * a));
      }

     // Косинус угла между векторами.

     inline friend double cos(const vector<N> &a, const vector<N> &b)
      {
         return(a * b / (fabs(a) * fabs(b)));
      }

     // Синус угла между векторами.

     inline friend double sin(const vector<N> &a, const vector<N> &b)
      {
         double x;
         x = cos(a, b);
         x *= x;
         return(sqrt(1. - x));
      }

     // Доступ к отдельным декартовым компонентам вектора.

     const double &operator[](int i) const
      {
         return components[i];
      }

     double &operator[](int i)
      {
         return components[i];
      }
    };


// Отдельно опишем типы для двумерных и трехмерных векторов.
// Будут дополнительно определены векторные произведения.

typedef vector<2> vector2D;

inline double operator%(const vector2D &a, const vector2D &b)
// Знаком % будем обозначать операцию векторного произведения.
{return(a[vector2D::X] * b[vector2D::Y] - a[vector2D::Y] * b[vector2D::X]);
}

inline vector2D operator~(const vector2D &r)
{// Поворот вектора на 90 градусов в положительном направлении.
 vector2D rezult;

 rezult[vector2D::X] = - r[vector2D::Y];
 rezult[vector2D::Y] = r[vector2D::X];
 return(rezult);
}

typedef vector<3> vector3D;

inline vector3D operator%(const vector3D &a, const vector3D &b)
{vector3D rezult;

 rezult[vector3D::X] = a[vector3D::Y] * b[vector3D::Z] -
                       b[vector3D::Y] * a[vector3D::Z];
 rezult[vector3D::Y] = a[vector3D::Z] * b[vector3D::X] -
                       b[vector3D::Z] * a[vector3D::X];
 rezult[vector3D::Z] = a[vector3D::X] * b[vector3D::Y] -
                       b[vector3D::X] * a[vector3D::Y];
 return(rezult);
}

#define vectorincluded

#endif

