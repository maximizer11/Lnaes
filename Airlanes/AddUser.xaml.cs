using Airlanes.DataSetAirlanesTableAdapters;
using System;
using System.Linq;
using System.Text;
using System.Windows;

namespace Airlanes
{
    /// <summary>
    /// Логика взаимодействия для AddUser.xaml
    /// </summary>
    public partial class AddUser : Window
    {
        OfficesTableAdapter officesTableAdapter = new OfficesTableAdapter();
        UsersTableAdapter usersTableAdapter = new UsersTableAdapter();
        UserViewTableAdapter userViewTableAdapter = new UserViewTableAdapter();
        Administrator main;
        public AddUser(Administrator main)
        {
            InitializeComponent();
            this.main = main;
            office.ItemsSource = officesTableAdapter.GetData();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            main.IsEnabled = true;
            Close();
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            StringBuilder errors = new StringBuilder();
            if (string.IsNullOrEmpty(firstName.Text))
            {
                errors.AppendLine("Укажите имя");
            }
            if (string.IsNullOrEmpty(lastName.Text))
            {
                errors.AppendLine("Укажите фамилию");
            }
            if (string.IsNullOrEmpty(password.Password))
            {
                errors.AppendLine("Укажите пароль");
            }
            if (string.IsNullOrEmpty(birthdate.Text))
            {
                errors.AppendLine("Укажите дату рождения");
            }
            if (string.IsNullOrEmpty(office.Text))
            {
                errors.AppendLine("Выберите офис");
            }
            if (string.IsNullOrEmpty(emailAddress.Text))
            {
                errors.AppendLine("Укажите email");
            }
            else
            {
                string email = emailAddress.Text;
                if (!email.Contains('@'))
                {
                    errors.AppendLine("Укажите верный формат email адреса");
                }
            }
            if (errors.Length > 0)
            {
                MessageBox.Show(errors.ToString());
            }
            else
            {
                string officename = office.Text;
                int idoffice = Convert.ToInt32(officesTableAdapter.ID(officename));
                usersTableAdapter.Save(2, emailAddress.Text, password.Password, firstName.Text, lastName.Text, idoffice, birthdate.Text, true);
                main.dataUsers.ItemsSource = userViewTableAdapter.GetData();
                main.IsEnabled = true;
                Close();
            }
        }
    }
}
