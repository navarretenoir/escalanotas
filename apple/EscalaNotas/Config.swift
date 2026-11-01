// Escala de Notas para macOS e iOS — configuración del anfitrión. Desarrollada por Javiera Navarrete.
import Foundation
extension ConfigApp {
    static let actual = ConfigApp(
        nombre: "Escala de Notas",
        inicio: .local("index.html"),
        puente: .escala,
        permitidos: [],
        fuera: ["http://", "https://"],
        ventanasDentro: false,
        activos: [])
}
