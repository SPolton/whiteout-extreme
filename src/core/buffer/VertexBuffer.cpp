#include "VertexBuffer.hpp"

#include <utility>

VertexBuffer::VertexBuffer(GLuint index, GLint size, GLenum dataType)
	: bufferID{}
	, attributeIndex(index)
	, attributeSize(size)
	, attributeType(dataType)
{
	// Just create the buffer, don't set up vertex attributes yet
	// Attributes should be configured when a VAO is properly bound
}

void VertexBuffer::uploadData(GLsizeiptr size, const void* data, GLenum usage) {
	bind();
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VertexBuffer::setupAttribute() {
	// Setup vertex attribute pointer - must be called with VAO bound
	bind();
	glVertexAttribPointer(attributeIndex, attributeSize, attributeType, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(attributeIndex);
}

IndexBuffer::IndexBuffer(GLuint /*index*/, GLint /*size*/, GLenum /*dataType*/)
    : bufferID{}
{
    // Just create the buffer
    // Index buffers don't need vertex attribute setup
}

void IndexBuffer::uploadData(GLsizeiptr size, const void* data, GLenum usage) {
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}
