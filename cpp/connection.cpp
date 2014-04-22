#include "connection.h"

hwo_connection::hwo_connection(const std::string& host, const std::string& port, const std::string& logname)
  : socket(io_service)
{
  tcp::resolver resolver(io_service);
  tcp::resolver::query query(host, port);
  boost::asio::connect(socket, resolver.resolve(query));
  response_buf.prepare(8192);
  rawlog.open("rawlog" + logname + ".txt");
}

hwo_connection::~hwo_connection()
{
  socket.close();
}

jsoncons::json hwo_connection::receive_response(boost::system::error_code& error)
{
  auto len = boost::asio::read_until(socket, response_buf, "\n", error);
  if (error)
  {
    return jsoncons::json();
  }
  auto buf = response_buf.data();
  std::string reply(boost::asio::buffers_begin(buf), boost::asio::buffers_begin(buf) + len);
  rawlog << "<< " << reply; // newline included
  response_buf.consume(len);
  return jsoncons::json::parse_string(reply);
}

void hwo_connection::send_requests(const std::vector<jsoncons::json>& msgs)
{
  jsoncons::output_format format;
  format.escape_all_non_ascii(true);
  boost::asio::streambuf request_buf;
  std::ostream s(&request_buf);
  for (const auto& m : msgs) {
    m.to_stream(s, format);
    s << std::endl;

    rawlog << ">> ";
    m.to_stream(rawlog, format);
    rawlog << std::endl;
  }
  socket.send(request_buf.data());
}
