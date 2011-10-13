#ifndef _PILLOW_HTTPHANDLER_H_
#define _PILLOW_HTTPHANDLER_H_

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QHash>

class QIODevice;
class QElapsedTimer;

namespace Pillow
{
	class HttpConnection;
	
	//
	// HttpHandler: abstract handler interface. Does nothing.
	//
	
	class HttpHandler : public QObject
	{
		Q_OBJECT
	
	public:
		HttpHandler(QObject *parent = 0);
	
	public slots:
		virtual bool handleRequest(Pillow::HttpConnection* connection) = 0;
	};
	
	//
	// HttpHandlerStack: a handler that delegates the request to zero or more children handlers until one handles the request.
	//
	
	class HttpHandlerStack : public HttpHandler
	{
		Q_OBJECT
		
	public:
		HttpHandlerStack(QObject* parent = 0);
				
	public:
		virtual bool handleRequest(Pillow::HttpConnection *connection);
	};

	//
	// HttpHandlerFixed: a handler that always returns the same specified response.
	//

	class HttpHandlerFixed : public HttpHandler
	{
		Q_OBJECT
		Q_PROPERTY(int statusCode READ statusCode WRITE setStatusCode NOTIFY changed)
		Q_PROPERTY(QByteArray content READ content WRITE setContent NOTIFY changed)
	
	private:
		int _statusCode;
		QByteArray _content;

	public:
		HttpHandlerFixed(int statusCode = 200, const QByteArray& content = QByteArray(), QObject* parent = 0);

		inline int statusCode() const { return _statusCode; }
		inline const QByteArray& content() const { return _content; }
		
	public:
		virtual bool handleRequest(Pillow::HttpConnection* connection);
		
	public slots:
		void setStatusCode(int statusCode);
		void setContent(const QByteArray& content);

	signals:
		void changed();
	};

	
	//
	// HttpHandler404: a handler that always respond "404 Not Found". Great as the last handler in your chain!
	//
	
	class HttpHandler404 : public HttpHandler
	{
		Q_OBJECT
		
	public:
		HttpHandler404(QObject* parent = 0);
		
	public:
		virtual bool handleRequest(Pillow::HttpConnection* connection);
	};
	
	//
	// HttpHandlerLog: a handler that logs completed requests.
	//
	
	class HttpHandlerLog : public HttpHandler
	{
		Q_OBJECT
		QHash<Pillow::HttpConnection*, QElapsedTimer*> _requestTimerMap;
		QPointer<QIODevice> _device;
		
	private slots:
		void requestCompleted(Pillow::HttpConnection* connection);
		void requestDestroyed(QObject* connection);
	
	public:
		HttpHandlerLog(QObject* parent = 0);
		HttpHandlerLog(QIODevice* device, QObject* parent = 0);
		~HttpHandlerLog();
		
		QIODevice* device() const;
		void setDevice(QIODevice* device);
		
	public:
		virtual bool handleRequest(Pillow::HttpConnection* connection);
	};
	
	//
	// HttpHandlerFile: a handler that serves static files from the filesystem.
	//
	
	class HttpHandlerFile : public HttpHandler
	{
		Q_OBJECT
		Q_PROPERTY(QString publicPath READ publicPath WRITE setPublicPath NOTIFY changed)
		
		QString _publicPath;
		int _bufferSize;
	
	public:
		HttpHandlerFile(const QString& publicPath = QString(), QObject* parent = 0);
		
		const QString& publicPath() const { return _publicPath; }
		int bufferSize() const { return _bufferSize; }
	
		enum { DefaultBufferSize = 512 * 1024 };
	
	public:
		void setPublicPath(const QString& publicPath);
		void setBufferSize(int bytes);
		
		virtual bool handleRequest(Pillow::HttpConnection* connection);
		
	signals:
		void changed();
	};
	
	class HttpHandlerFileTransfer : public QObject
	{
		Q_OBJECT
		QPointer<QIODevice> _sourceDevice;
		QPointer<HttpConnection> _connection;
		int _bufferSize;
	
	public:
		HttpHandlerFileTransfer(QIODevice* sourceDevice, Pillow::HttpConnection* connection, int bufferSize = HttpHandlerFile::DefaultBufferSize);
	
	public slots:
		void writeNextPayload();
	
	signals:
		void finished();
	};
}
#endif // _PILLOW_HTTPHANDLER_H_
