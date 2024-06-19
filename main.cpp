/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AliceInNov <alice@alicenov.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/01 23:32:04 by AliceInNov        #+#    #+#             */
/*   Updated: 2024/06/20 03:32:09 by AliceInNov       ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "Classes/Server.hpp"
#include "Includes/utils.hpp"

int main()
{
	try {
		Server myServer(25000);
		myServer.ServerLoop();
	}
	catch (const std::string& e) {
		PrintError(e);
	}
	catch (const std::exception& e) {
		PrintError("ERROR  : std::exception , " + std::string(e.what()));
	}
	catch (...) {
		PrintError("ERROR  : Unknown, ServerLoop()");
	}
}