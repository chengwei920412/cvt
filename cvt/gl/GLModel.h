#ifndef CVT_GLMODEL_H
#define CVT_GLMODEL_H

#include <cvt/geom/Model.h>
#include <cvt/gl/GLBuffer.h>
#include <cvt/gl/GLVertexArray.h>


namespace cvt
{
	class GLModel
	{
		public:
			GLModel();
			GLModel( const Model& mdl );
			~GLModel();

			void setColor( const Color& col );
			void draw() const;
		private:
			GLVertexArray _vao;
			GLBuffer	  _vtxbuf;
			GLBuffer	  _normbuf;
			GLBuffer	  _facebuf;
			size_t		  _numvertices;
			size_t		  _numfaces;
			GLenum		  _mode;
	};

	inline void GLModel::draw() const
	{
		_vao.drawIndirect( _facebuf, GL_UNSIGNED_INT, _mode, _numfaces * 3 );
	}

	inline void GLModel::setColor( const Color& col )
	{
		_vao.setColor( col );
	}

}

#endif