using System;
using System.Linq;
using System.Text;
using System.Windows;
using Airlanes.DataSetAirlanesTableAdapters;
namespace Airlanes
{
    /// <summary>
    /// Логика взаимодействия для EditUser.xaml
    /// </summary>
    public partial class EditUser : Window
    {
        OfficesTableAdapter officesTableAdapter = new OfficesTableAdapter();
        UsersTableAdapter usersTableAdapter = new UsersTableAdapter();
        UserViewTableAdapter userViewTableAdapter = new UserViewTableAdapter();
        Administrator main;
        int id;
        public EditUser(Administrator main, string id)
        {
            InitializeComponent();
            this.main = main;
            this.id = Convert.ToInt32(id);
            comboOffice.ItemsSource = officesTableAdapter.GetData();
            string emaill = usersTableAdapter.Email(Convert.ToInt32(id));
            emailAddress.Text = emaill;
            string first = usersTableAdapter.FirstName(Convert.ToInt32(id));
            firstName.Text = first;
            string last = usersTableAdapter.LastName(Convert.ToInt32(id));
            lastName.Text = last;
            int? officee = usersTableAdapter.Office(Convert.ToInt32(id));
            string office_title = officesTableAdapter.Title(Convert.ToInt32(officee));
            comboOffice.Text = office_title;
        }

        private void cancel_Click(object sender, RoutedEventArgs e)
        {
            main.IsEnabled = true;
            Close();
        }

        private void apply_Click(object sender, RoutedEventArgs e)
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
            if (string.IsNullOrEmpty(comboOffice.Text))
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
                string officename = comboOffice.Text;
                int idoffice = Convert.ToInt32(officesTableAdapter.ID(officename));
                usersTableAdapter.UpdateUser(2, emailAddress.Text, firstName.Text, lastName.Text, idoffice, id);
                main.dataUsers.ItemsSource = userViewTableAdapter.GetData();
                main.IsEnabled = true;
                Close();
            }
        }
    }
}
