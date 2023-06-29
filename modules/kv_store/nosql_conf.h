// /**************************************************************************/
// /*  nosql_conf.h                                               */
// /**************************************************************************/
// /*                         This file is part of:                          */
// /*                             GODOT ENGINE                               */
// /*                        https://godotengine.org                         */
// /**************************************************************************/
// /* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
// /* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
// /*                                                                        */
// /* Permission is hereby granted, free of charge, to any person obtaining  */
// /* a copy of this software and associated documentation files (the        */
// /* "Software"), to deal in the Software without restriction, including    */
// /* without limitation the rights to use, copy, modify, merge, publish,    */
// /* distribute, sublicense, and/or sell copies of the Software, and to     */
// /* permit persons to whom the Software is furnished to do so, subject to  */
// /* the following conditions:                                              */
// /*                                                                        */
// /* The above copyright notice and this permission notice shall be         */
// /* included in all copies or substantial portions of the Software.        */
// /*                                                                        */
// /* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
// /* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
// /* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
// /* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
// /* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
// /* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
// /* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
// /**************************************************************************/


#ifndef NOSQL_DB_NOSQL_CONF_H
#define NOSQL_DB_NOSQL_CONF_H

#include "core/variant/variant.h"
#include "core/io/config_file.h"

class NoSQLConfig {
public:
	NoSQLConfig(const String & p_config_file_path = "user://nosql.conf");
	~NoSQLConfig();
	void set_database_dir(const String & p_database_dir);
	String get_database_dir() const;
	void add_database_config(const String & p_database_name, const Dictionary & p_config);
	Dictionary get_database_config(const String & p_database_name) const;

private:
	Ref<ConfigFile> _config_file;
	String _config_file_path;
	void _write_default_config();
};

#endif
